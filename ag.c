
#include "agnostic.h"
#include "run-cmd.h"

#include <sys/param.h>
#include <sys/wait.h>
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

static void clone() { 
    char* cfg_file = create_config_file_name();
    if (!cfg_file) {
        die("Config file not found");
    }

    struct ag_project* project = NULL;
    if (ag_load(cfg_file, &project)) {
        die("Failed to load the project");
    }

    struct ag_component_list* l = project->components;
    int i = 0;
    int* pids = (int*)malloc(sizeof(int) * project->component_count);
    sds* names = (sds*)malloc(sizeof(sds) * project->component_count);
    sds* cmdlines = (sds*)malloc(sizeof(sds) * project->component_count);
    sds* aliases = (sds*)malloc(sizeof(sds) * project->component_count);
    struct ag_component* c;
    while (l) {
        c = l->component;

        const char* vcs_exe = NULL;
        if (c->git) {
            vcs_exe = "git";
        } else if (c->hg) {
            vcs_exe = "hg";
        } else {
            fprintf(stderr, "Unknown VCS for %s\n", c->name);
            continue;
        }
        sds vcs = sdsdup(c->git ? c->git : c->hg);
        sds cmdline = sdscatprintf(sdsempty(), "%s clone \"%s\" \"%s\"", vcs_exe, vcs, c->name);

        printf("Starting cloning %s\n", c->name);
        int child_pid = run_cmd_line(cmdline);
        if (-1 == child_pid) {
            perror(NULL);
            fprintf(stderr, "Failed to run clone for %s\n", c->name);
            exit(1);
        }

        pids[i] = child_pid;
        names[i] = c->name;
        cmdlines[i] = cmdline;
        aliases[i] = c->alias;

        l = l->next;
        ++i;
    }

    int process_left = project->component_count;
    int status = 0;
    while (0 < process_left) {
        pid_t pid = wait(&status);
        --process_left;
        for (int i = 0; i < project->component_count; ++i) {
            if (pids[i] == pid) {
                if (WIFEXITED(status)) {
                    if (WEXITSTATUS(status)) {
                        printf("Failed to clone %s. Please, run it manually:\n    %s\n", names[i], cmdlines[i]);
                    } else {
                        printf("Successfully cloned %s\n", names[i]);
                        if (aliases[i]) {
                            if (symlink(names[i], aliases[i])) {
                                perror(NULL);
                                fprintf(stderr, "Failed to create alias symlink for %s\n", names[i]);
                            }
                        }
                    }
                } else {
                    printf("Stopped cloning %s\n", names[i]);
                }
                break;
            }
        }
    }

    free(pids);
    free(names);
    for (int i = 0; i < project->component_count; ++i) {
        sdsfree(cmdlines[i]);
    }
    free(cmdlines);
    ag_free(project);
    free(cfg_file);

    exit(0);
}

static void help() {
    printf("%s\n%s\n", 
        "ag-info <command>", 
        "Recognized commands: vcs, config-file, help.");
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

        if (!strcmp(cmd, "clone")) {
            clone();

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
