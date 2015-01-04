
#include "agnostic.h"
#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern void clone();
extern void component();

static void help() {
    printf("%s\n%s\n", 
        "ag-info <command>", 
        "Recognized commands: clone, project, help.");
}

static void unknown_cmd(const char* cmd) {
    die("Unknown command: %s", cmd);
}

static void project_file() {
    char f[PATH_MAX + 1];
    if (ag_find_project_file(f)) {
        die("Project file not found");
    }
    printf("%s\n", f);
    exit(0);
}

int main(int argc, char **av) {
    const char **argv = (const char **) av;

    if (1 >= argc) {
        help();
    }

    while (1 < argc) {
        --argc;
        ++argv;

        const char *cmd = *argv;

        if (!strcmp(cmd, "clone")) {
            clone();

        } else if (!strcmp(cmd, "component")) {
            component();
            
        } else if (!strcmp(cmd, "project")) {
            project_file();
            
        } else if (!strcmp(cmd, "help")) {
            help();

        } else {
            unknown_cmd(cmd);
        }
    }

    return 0;
}
