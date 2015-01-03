
#include "agnostic.h"
#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern void clone();

static void help() {
    printf("%s\n%s\n", 
        "ag-info <command>", 
        "Recognized commands: clone, project, help.");
}

static void unknown_cmd(const char* cmd) {
    die("Unknown command: %s", cmd);
}

static void project_file() {
    char* f = ag_create_project_file_name();
    if (!f) {
        fprintf(stderr, "Project file not found\n");
        exit(1);
    }
    printf("%s\n", f);
    free(f);
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
