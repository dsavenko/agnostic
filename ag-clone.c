
#include "agnostic.h"
#include "common.h"

#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

static void checked_symlink(const char* name, const char* alias, int check_exist) {
    if (empty(alias) || empty(name)) {
        return;
    }
    if (symlink(name, alias)) {
        if (check_exist || EEXIST != errno) {
            perror(NULL);
            fprintf(stderr, "Failed to create alias symlink %s -> %s\n", alias, name);
        }
    }
}

void clone(int argc, const char** argv) { 
    struct ag_project* project = ag_load_default_or_die();
    struct ag_component_list* l = project->components;
    int i = 0;
    pid_t* pids = (pid_t*)xmalloc(sizeof(pid_t) * project->component_count);
    char** names = (char**)xmalloc(sizeof(char*) * project->component_count);
    char** cmdlines = (char**)xmalloc(sizeof(char*) * project->component_count);
    char** aliases = (char**)xmalloc(sizeof(char*) * project->component_count);
    struct ag_component* c;
    while (l) {
        c = l->component;

        if (dir_exists(c->name)) {
            printf("Looks like component is already cloned: %s\n", c->name);
            checked_symlink(c->name, c->alias, 0);
            l = l->next;
            continue;
        }

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

    int process_size = i;
    int process_left = process_size;
    int status = 0;
    while (0 < process_left) {
        pid_t pid = wait(&status);
        --process_left;
        for (int i = 0; i < process_size; ++i) {
            if (pids[i] == pid) {
                if (WIFEXITED(status)) {
                    if (WEXITSTATUS(status)) {
                        printf("Failed to clone %s. Please, run it manually:\n    %s\n", names[i], cmdlines[i]);
                    } else {
                        printf("Successfully cloned %s\n", names[i]);
                        checked_symlink(names[i], aliases[i], 1);
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
    for (int i = 0; i < process_size; ++i) {
        free(cmdlines[i]);
    }
    free(cmdlines);
    ag_free(project);
}
