
#include "agnostic.h"
#include "common.h"

#include <sys/param.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void clone() { 
    struct ag_project* project = NULL;
    if (ag_load_default(&project)) {
        die("Failed to load project");
    }

    struct ag_component_list* l = project->components;
    int i = 0;
    pid_t* pids = (pid_t*)malloc(sizeof(pid_t) * project->component_count);
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
        sdsfree(cmdlines[i]);
    }
    free(cmdlines);
    ag_free(project);

    exit(0);
}
