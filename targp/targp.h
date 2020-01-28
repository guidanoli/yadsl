#ifndef __TESTW_H__
#define __TESTW_H__

// Command structure
typedef struct
{
    const char *cmd;
    int argc;
    char *(*parse_cb)(int argc, char **argv);
}
Command;

// parse_cb prototype macro
#define DEFPARSECB(label) static char * _ ## label (int argc, char **argv);

// Command definition macro
#define DEFCMD(label, argc) { #label , argc, _ ## label }

// Command array sentinel macro
#define END_CMDS { NULL, NULL, NULL }

/**
* Parse test arguments to TARGP
* argc, argv    parameters passed to main function
* helpStrings   array of strings terminated with NULL
* cmds          array of test commands terminated with END_CMDS
* error_cb      function called when an error is detected
*/
int targp(int argc, char **argv, const char **helpStrings, Command *cmds,
    void (*error_cb)(int argi, char *arg, char *errorMessage));

#endif