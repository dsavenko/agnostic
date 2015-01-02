
#include "agnostic.h"

#include <sys/param.h>
#include <unistd.h>
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

static bool file_exist(const char* fname) {
    return access(fname, F_OK) != -1;
}

static char* create_config_file_name() {
    const int size = 2;
    const char* files[size] = { "agnostic.yaml", "../agnostic.yaml" };
    const char* relative = NULL;
    for (int i = 0; i < 2; ++i) {
        if (file_exist(files[i])) {
            relative = files[i];
            break;
        }
    }

    if (!relative) {
        return NULL;
    }

    char* absolute = (char*)malloc(sizeof(char) * (PATH_MAX+1));
    if (realpath(relative, absolute)) {
        return absolute;
    }

    free(absolute);
    return NULL;
}

static void checkout() { 
    char* cfg_file = create_config_file_name();
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
    free(cfg_file);

    exit(0);
}

static void help() {
    printf("%s\n", "help");
}

static void unknown_cmd(const char* cmd) {
    die("Unknown command: %s", cmd);
}

static void config_file() {
    char* f = create_config_file_name();
    if (!f) {
        fprintf(stderr, "Config file not found\n");
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

        if (!strcmp(cmd, "checkout")) {
            checkout();

        } else if (!strcmp(cmd, "config-file")) {
            config_file();
            
        } else if (!strcmp(cmd, "help")) {
            help();

        } else {
            unknown_cmd(cmd);
        }
    }

    return 0;
}
