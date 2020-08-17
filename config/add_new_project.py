'''Adds a new project to the repository'''

def get_repository_root():
    '''Get repository root based on this file's path
    
    Return:
        repository root (pathlib.Path)
    '''
    from pathlib import Path
    return (
        Path(__file__)  # root/config/add_new_project.py
        .resolve()      # root/config/
        .parents[1]     # root
    )                   # [1]   [0]

class BaseFileEntity(object):
    '''A generic target

    Parameters:
        make_if - return value for can_make (bool)
            Default: True
    '''
    def __init__(self, *args, **kwargs):
        self.make_if = kwargs.get('make_if', True)
    def can_make(self, *args, **kwargs):
        return self.make_if
    def make(self, *args, **kwargs):
        pass

class File(BaseFileEntity):
    '''A file in system

    Parameters:
        name    - file name and extension (str)
        make_cb - callback called with file pointer (function)
            Default: None (empty file)
        append  - whether make_cb will append or write to file (bool)
            Default: False (writes from BOF)
    '''
    def __init__(self, name, make_cb=None, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.name = name
        self.make_cb = make_cb
        self.append = kwargs.get('append', False)
    def make(self, *args, **kwargs):
        prj = kwargs['name']
        file_path = (
            kwargs['path']      # C:/aa/src/?
            / self.name         # C:/aa/src/?/?.h
            .__str__()          # C:\aa\src\?\?.h
            .replace('?', prj)  # C:\aa\src\prj\prj.h
        )
        mode = 'a' if self.append else 'w'
        with open(file_path, mode) as f:
            if self.make_cb:
                self.make_cb(f, *args, **kwargs)

class Group(BaseFileEntity):
    '''A group of entities

    Parameters:
        items - collection of entities (list)
    '''
    def __init__(self, items, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.items = items
    def make(self, *args, **kwargs):
        for item in self.items:
            if item.can_make(*args, **kwargs):
                item.make(*args, **kwargs)

class Directory(Group):
    '''
    A directory in system

    Parameters:
        name  - directory name (str)
        items - directory items (list)
        new   - whether directory must be created (bool)
            Default: True (creates directory)
    '''
    def __init__(self, name, items, *args, **kwargs):
        super().__init__(items, *args, **kwargs)
        self.name = name
        self.new = kwargs.get('new', True)
    def make(self, *args, **kwargs):
        import os
        prj = kwargs['name']
        realname = (
            self.name
            .replace('?', prj)
        )
        kwargs['path'] /= realname
        if self.new:
            os.mkdir(kwargs['path'])
        super().make(*args, **kwargs)

class TemplateSubstitutionRule(object):
    '''Substitution rule

    Parameters:
        pattern - re pattern to be matched
        subfunc - function called by re.sub
    '''
    def __init__(self, pattern, subfunc):
        self.pattern = pattern
        self.subfunc = subfunc
    def substitute(self, text):
        while True:
            newtext = self.pattern.sub(self.subfunc, text)
            if newtext == text:
                break
            text = newtext
        return text

class TemplateSubstitutionRuleSet(TemplateSubstitutionRule):
    '''Substitution rule set

    Parameters:
        rules - collection of substitution rules (list)
    '''
    def __init__(self, rules):
        self.rules = rules
    def substitute(self, text):
        for rule in self.rules:
            text = rule.substitute(text)
        return text

class FileFromTemplate(File):
    '''A file created from template with substitution rules

    Parameters:
        name     - same as File.name
        template - template file path
        rule     - substitution rule
    '''
    def __init__(self, name, template, rule, *args, **kwargs):
        super().__init__(name, self.write, *args, **kwargs)
        self.template = template
        self.rule = rule
    def write(self, f, *args, **kwargs):
        with open(self.template, 'r') as tf:
            text = tf.read()
            newtext = self.rule.substitute(text)
            f.write(newtext)

def main(*args, **kwargs):
    '''Creates project to the code base
    
    main(project, **opts)
    
    Parameters:
        project - project name (str)
        
    Optional parameters:
        test - add test suite (bool)
            Default: False
        python - add Python binding (bool)
            Default: False
        lua - add lua binding (bool)
            Default: False
    '''
    import os
    import re
    
    kwargs['name'] = args[0]
    
    var_patt = re.compile(r"(%[^%]+%)")
    cond_patt = re.compile(r"<([^?]+)\?([^:]*):([^>]*)>")
    
    def var_replace(match):
        key = match.group(0)[1:-1] # removes %%
        err = f'<missing value for {key}>'
        return kwargs.get(key, err)

    def cond_replace(match):
        cond, true, false = match.groups()
        try:
            val = eval(cond, {}, kwargs)
        except:
            val = False
        return true if val else false

    var_rule = TemplateSubstitutionRule(var_patt, var_replace)
    cond_rule = TemplateSubstitutionRule(cond_patt, cond_replace)
    rule_set = TemplateSubstitutionRuleSet([cond_rule, var_rule])

    make_test = kwargs.get('test', False)
    make_python = kwargs.get('python', False)
    make_lua = kwargs.get('lua', False)
   
    root = get_repository_root()

    class ProjectFileFromTemplate(FileFromTemplate):
        def __init__(self, filepath, *args, **kwargs):
            import re
            templatepath = (
                root
                / 'config'
                / 'templates'
                / 'src'
                / 'x'
                / filepath.replace('?', 'x')
                .__str__()
            )
            filename = re.sub('(.*/)', '', filepath)
            super().__init__(filename, templatepath, rule_set, *args, **kwargs)
    
    directory = Directory('src', [
        Directory('?', [
            ProjectFileFromTemplate('CMakeLists.txt'),
            File('?.c'),
            File('?.h'),
            Group([
                ProjectFileFromTemplate('?.test.c'),
                File('?.script'),
            ], make_if=make_test),
            Directory('python', [
                ProjectFileFromTemplate('python/CMakeLists.txt'),
                ProjectFileFromTemplate('python/?.py.c'),
                ProjectFileFromTemplate('python/?_test.py'),
            ], make_if=make_python),
            Directory('lua', [
                ProjectFileFromTemplate('lua/CMakeLists.txt'),
                ProjectFileFromTemplate('lua/?.lua.c'),
                File('?_test.lua'),
            ], make_if=make_lua),
        ]),
        ProjectFileFromTemplate('../CMakeLists.txt', append=True)
    ], new=False)
    
    directory.make(*args, **kwargs, path=root)

def process_argv(argv):
    '''Process command-line arguments into args and kwargs

    --key=value => ['key'] = eval(value, {}, kwargs)
    --key       => ['key'] = True
    -abc        => ['a'] = ['b'] = ['c'] = True
    value       => args.append(eval(value, {}, kwargs))

    Parameters:
        argv - command line arguments
               Hint: see sys.argv
               Hint: don't forget to remove sys.argv[0]
    Return:
        args   - list of positional arguments
        kwargs - dictionary of keyword arguments
    '''
    import re

    args = list()
    kwargs = dict()
    
    def _eval(exp):
        try:
            return eval(exp, {}, kwargs)
        except:
            return exp
    
    kw_patt = re.compile(r'^--([^=]+)=(.*)$')
    string_flag_patt = re.compile(r'^--([^=]+)$')
    char_flag_patt = re.compile(r'^-([^=]+)$')

    for arg in argv:
        if match := kw_patt.match(arg):
            k, v = match.groups()
            kwargs[k] = _eval(v)
        elif match := string_flag_patt.match(arg):
            v = match.group(1)
            kwargs[v] = True
        elif match := char_flag_patt.match(arg):
            vs = match.group(1)
            for v in vs:
                kwargs[v] = True
        else:
            v = _eval(arg)
            args.append(v)
    
    return args, kwargs

__usage__ = f'''Usage

  python {__file__} <project-name> [options]

Since the project files will reside in a folder of same name, it must contain
only valid characters, as for any other folder in your operating system.'''

__usage_help__ = f'''Run 'python {__file__} --help' for more information.'''

__usage_opts__ = f'''Options
  --help=<bool>    = Print usage information and exit.
  --test=<bool>    = Create C test module.
  --python=<bool>  = Create Python binding.
  --lua=<bool>     = Create Lua binding.'''

__usage_eval__ = f'''Argument evaluation
  Command line arguments are evaluated in a special way.'''

__usage_eval_args__ = f'''  Positional arguments
    <expr>           = Evaluation of <expr> as Python expression.
    <not-expr>       = Evaluation of <not-expr> as string.'''

__usage_eval_kwargs__ = f'''  Keyword arguments
    --key=<expr>     = Evaluation of <expr> as Python expression.
    --key=<not-expr> = Evaluation of <not-expr> as string.
    --key            = --key=True
    -key             = --k --e --y'''

__usage_eval_strings__ = f'''  Evaluating arguments as strings
    By using quotes (") or ('), the argument is forced to evaluate to a string.
    For example, take the word 'list'.
    
    list             = <class 'list'>
    'list'           = 'list' '''

if __name__ == '__main__':
    import sys
    args, kwargs = process_argv(sys.argv[1:])
    if kwargs.get('help', False):
        exit('\n\n'.join([__usage__,
                          __usage_opts__,
                          __usage_eval__,
                          __usage_eval_args__,
                          __usage_eval_kwargs__,
                          __usage_eval_strings__]))
    try:
        main(*args, **kwargs)
    except Exception as e:
        print("Error:", e)
        exit('\n\n'.join([__usage__,
                          __usage_help__]))
