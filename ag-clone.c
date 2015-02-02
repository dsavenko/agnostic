
#include "agnostic.h"

#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define START_COLOR TERM_COLOR_CYAN
#define FINISH_COLOR TERM_COLOR_GREEN

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

static int already_cloned(struct ag_component* c) {
    if (dir_exists(c->name)) {
        printf(FINISH_COLOR "Looks like component is already cloned: %s" TERM_COLOR_RESET "\n", c->name);
        checked_symlink(c->name, c->alias, 0);
        return 1;
    } else {
        return 0;
    }
}

static char* create_cmdline(struct ag_component* c) {
    const char* vcs_exe = NULL;
    if (c->git) {
        vcs_exe = "git";
    } else if (c->hg) {
        vcs_exe = "hg";
    } else {
        die("Unknown VCS for %s\n", c->name);
    }
    char* vcs = c->git ? c->git : c->hg;
    char* cmdline = NULL;
    if (-1 == asprintf(&cmdline, "%s clone \"%s\" \"%s\"", vcs_exe, vcs, c->name)) {
        die("Couldn't allocate memory for command line");
    }
    return cmdline;
}

static int run_clone_or_die(struct ag_component* c, const char* cmdline, int supress_output) {
    printf(START_COLOR "Starting cloning %s" TERM_COLOR_RESET "\n", c->name);
    int child_pid = run_cmd_line(cmdline, supress_output);
    if (-1 == child_pid) {
        perror(NULL);
        fprintf(stderr, "Failed to run clone for %s\n", c->name);
        xexit(1);
    }
    return child_pid;
}

static int finish_cloning(int status, const char* name, const char* alias, const char* cmdline) {
    if (WIFEXITED(status)) {
        if (WEXITSTATUS(status)) {
            printf("Failed to clone %s. Please, run it manually:\n    %s\n", name, cmdline);
        } else {
            printf(FINISH_COLOR "Successfully cloned %s" TERM_COLOR_RESET "\n", name);
            checked_symlink(name, alias, 1);
            return 0;
        }
    } else {
        printf("Stopped cloning %s\n", name);
    }
    return 1;
}

static void clone_sequential() {
    struct ag_project* project = ag_load_default_or_die();
    struct ag_component* c;
    for (struct list* l = project->components; l; l = l->next) {
        c = (struct ag_component*)l->data;

        if (already_cloned(c)) {
            continue;
        }

        char* cmdline = create_cmdline(c);
        run_clone_or_die(c, cmdline, 0);

        int status = 0;
        wait(&status);
        finish_cloning(status, c->name, c->alias, cmdline);

        free(cmdline);
    }
    ag_free(project);
}

static void clone_parallel() {
    struct ag_project* project = ag_load_default_or_die();
    int i = 0;
    pid_t* pids = (pid_t*)xmalloc(sizeof(pid_t) * project->component_count);
    char** names = (char**)xmalloc(sizeof(char*) * project->component_count);
    char** cmdlines = (char**)xmalloc(sizeof(char*) * project->component_count);
    char** aliases = (char**)xmalloc(sizeof(char*) * project->component_count);
    struct ag_component* c;
    for (struct list* l = project->components; l; l = l->next) {
        c = (struct ag_component*)l->data;

        if (already_cloned(c)) {
            continue;
        }

        char* cmdline = create_cmdline(c);
        int child_pid = run_clone_or_die(c, cmdline, 1);

        pids[i] = child_pid;
        names[i] = c->name;
        cmdlines[i] = cmdline;
        aliases[i] = c->alias;

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
                finish_cloning(status, names[i], aliases[i], cmdlines[i]);
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

static void download_project_file(const char* url) {
    printf(START_COLOR "Downloading project file" TERM_COLOR_RESET "\n");
    char* cmdline = NULL;
    asprintf(&cmdline, "curl -sS -o agnostic.yaml \"%s\"", url);
    run_cmd_line(cmdline, 0);
    int status = 0;
    wait(&status);
    if (WIFEXITED(status)) {
        if (WEXITSTATUS(status)) {
            die("Failed to download %s. Please, download it manually, then run 'ag clone'\n", url);
        }
    } else {
        die("Stopped downloading %s\n", url);
    }
    free(cmdline);
}

void clone(int argc, const char** argv) {
    int parallel = 0;
    const char* url = NULL;

    while (0 < argc) {
        if (!strcmp("-p", *argv) || !strcmp("--parallel", *argv)) {
            parallel = 1;
        } else if (0 < strlen(*argv) && '-' == (*argv)[0]) {
            die("Unrecognized option: %s", *argv);
        } else if (!url) {
            url = *argv;
        } else {
            die("Unrecognized argument: %s", *argv);
        }

        --argc;
        ++argv;
    }

    if (url) {
        download_project_file(url);
    }

    if (parallel) {
        clone_parallel();
    } else {
        clone_sequential();
    }
}
