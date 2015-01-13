
#include "agnostic.h"
#include "common.h"

#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void clone(int argc, const char** argv) { 
    struct ag_project* project = ag_load_default_or_die();
    struct ag_component_list* l = project->components;
    int i = 0;
    pid_t* pids = (pid_t*)malloc(sizeof(pid_t) * project->component_count);
    char** names = (char**)malloc(sizeof(char*) * project->component_count);
    char** cmdlines = (char**)malloc(sizeof(char*) * project->component_count);
    char** aliases = (char**)malloc(sizeof(char*) * project->component_count);
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
        char* vcs = c->git ? c->git : c->hg;
        char* cmdline = NULL;
        if (-1 == asprintf(&cmdline, "%s clone \"%s\" \"%s\"", vcs_exe, vcs, c->name)) {
            die("Couldn't allocate memory for command line");
        }

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
                                fprintf(stderr, "Failed to create alias symlink %s -> %s\n", aliases[i], names[i]);
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
        free(cmdlines[i]);
    }
    free(cmdlines);
    ag_free(project);
}
