'''Adds new project to the code base

When run directly, additional parameters are
processed in the following way:

- If contains '=', it is split between key and value
  and added as a keyword argument
    - key is a string
    - value is evaluated
- If doesn't contain '=', it is evaluated and added
  as a positional argument (in the same order)

PS: If evaluation fails, it is used as string

Example:

python add_new_project prj python=True lua=False

... calls main('prj', python=True, lua=False)
'''

def get_repository_root():
    '''
    Get repository root based on this file's path
    
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
    def __init__(self, *args, **kwargs):
        self.make_if = kwargs.get('make_if', True)
    def can_make(self, *args, **kwargs):
        return self.make_if
    def make(self, *args, **kwargs):
        pass

class File(BaseFileEntity):
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
    def __init__(self, items, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.items = items
    def make(self, *args, **kwargs):
        for item in self.items:
            if item.can_make(*args, **kwargs):
                item.make(*args, **kwargs)

class Directory(Group):
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
    def __init__(self, rules):
        self.rules = rules
    def substitute(self, text):
        for rule in self.rules:
            text = rule.substitute(text)
        return text

class FileFromTemplate(File):
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
    '''
    Creates project to the code base
    
    main(project, **opts)
    
    Parameters:
        project - project name (str)
        
    Optional parameters:
        tests - add test suites (bool)
            Default: False
        python - add Python bindings (bool)
            Default: False
        lua - add lua bindings (bool)
            Default: False
    '''
    import os
    import re
    kwargs['name'] = args[0]
    
    var_patt = re.compile("(%[^%]+?%)")
    cond_patt = re.compile("<([^?]+)\?([^:]*):([^>]*)>")
    
    def var_replace(match):
        key = match.group(0)[1:-1]
        err = f'<missing value for {key}>'
        return kwargs.get(key, err)

    def cond_replace(match):
        cond_exp, true_val, false_val = match.groups()
        try:
            val = eval(cond_exp, {}, kwargs)
        except:
            val = False
        return true_val if val else false_val

    var_rule = TemplateSubstitutionRule(var_patt, var_replace)
    cond_rule = TemplateSubstitutionRule(cond_patt, cond_replace)
    rule_set = TemplateSubstitutionRuleSet([cond_rule, var_rule])

    make_tests = kwargs.get('tests', False)
    make_python = kwargs.get('python', False)
    make_lua = kwargs.get('lua', False)
    
    class ProjectFileFromTemplate(FileFromTemplate):
        def __init__(self, name, *args, **kwargs):
            import re
            template = (
                'templates'
                +os.path.sep
                +'src'
                +os.path.sep
                +'x'
                +os.path.sep
                +name.replace('?', 'x')
            )
            patt = re.compile('(.*/)')
            name = patt.sub('', name)
            super().__init__(name, template, rule_set, *args, **kwargs)
    
    directory = Directory('src', [
        Directory('?', [
            ProjectFileFromTemplate('CMakeLists.txt'),
            File('?.c'),
            File('?.h'),
            Group([
                ProjectFileFromTemplate('?.test.c'),
                File('?.script'),
            ], make_if=make_tests),
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
    
    directory.make(*args, **kwargs, path=get_repository_root())

if __name__ == '__main__':
    import sys
    args = list()
    kwargs = dict()
    
    def _eval(exp):
        try:
            return eval(exp, {}, kwargs)
        except:
            return exp
        
    for arg in sys.argv[1:]:
        if '=' in arg:
            k, v = arg.split('=')
            kwargs[k] = _eval(v)
        else:
            v = _eval(arg)
            args.append(v)
    
    main(*args, **kwargs)
