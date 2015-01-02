
#include "agnostic.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void die (const char * format, ...) {
    va_list vargs;
    va_start (vargs, format);
    vfprintf (stderr, format, vargs);
    fprintf (stderr, "\n");
    exit (1);
}

static const char* find_config_file() {
    return "ez/agnostic.yaml";
}

static int checkout() { 
    const char* cfg_file = find_config_file();
    if (!cfg_file) {
        die("Config file not found");
    }

    struct ag_project* project = NULL;
    if (ag_load(cfg_file, &project)) {
        die("Failed to load the project");
    }

    struct ag_component_list* c = project->components;
    while (c) {
        printf("%s\n", c->component->name);
        c = c->next;
    }

    ag_free(project);
    return 0;
}

static void help() {
    printf("%s\n", "help");
}

static void unknown_cmd(const char* cmd) {
    die("Unknown command: %s", cmd);
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

        if (!strcmp(cmd, "checkout")) {
            checkout();
            exit(0);
        } else if (!strcmp(cmd, "help")) {
            help();
        } else {
            unknown_cmd(cmd);
        }
    }

    return 0;
}
