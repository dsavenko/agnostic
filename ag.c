
#include "agnostic.h"
#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct cmd_struct {
    const char* name;
    const char* shortcut;
    void (*fn)(int argc, const char** argv);
};

extern void clone(int argc, const char** argv);
extern void component(int argc, const char** argv);
extern void build(int argc, const char** argv);
extern void project(int argc, const char** argv);

static void help(int argc, const char** argv);

static struct cmd_struct commands[] = {

        // functions
        { "clone", "", &clone },
        { "component", "comp", &component },
        { "project", "proj", &project },
        { "build", "", &build },
        { "help", "", &help },

        // scripts
        { "remove", "", NULL }
    };

static void help(int argc, const char** argv) {
    printf("%s\n%s", "ag <command>", "Recognized commands: ");
    for (int i = 0; i < ARRAY_SIZE(commands); ++i) {
        struct cmd_struct* c = commands + i;
        printf("%s", c->name);
        if (c->shortcut && c->shortcut[0]) {
            printf("/%s", c->shortcut);
        }
        printf(" ");
    }
    printf("\n");
}

static void setup_path(const char* exec_path) {
    if (!strchr(exec_path, '/')) {
        // assume, that PATH is valid
        return;
    }

    char* absolute = realpath(exec_path, NULL);
    char* parent = parent_dir(absolute);

    const char *old_path = getenv("PATH");
    char* new_path = NULL;
    if (old_path) {
        if (-1 == asprintf(&new_path, "%s:%s", parent, old_path)) {
            die("Out of memory, asprintf failed");
        }
        free(parent);
    } else {
        new_path = parent;
    }
    if (setenv("PATH", new_path, 1)) {
        die("Unable to set PATH");
    }
    free(absolute);
    free(new_path);
}

static void run_external_cmd(struct cmd_struct* cmd, int argc, const char** argv) {
    char* script = NULL;
    if (-1 == asprintf(&script, "ag-%s.sh", cmd->name)) {
        die("Out of memory, asprintf failed");
    }
    char* exec_argv[argc + 2];
    exec_argv[0] = script;
    if (0 < argc) {
        memcpy(exec_argv + 1, argv, argc * sizeof(const char*));
    }
    exec_argv[argc + 1] = NULL;
    execvp(script, exec_argv);
    perror(NULL);
    die("Unable to run script");
}

int main(int argc, char **av) {
    const char **argv = (const char **) av;

    if (1 >= argc) {
        help(0, NULL);
        return 0;
    }

    const char* exec_path = *argv;

    --argc;
    ++argv;
    const char *cmd = *argv;

    --argc;
    ++argv;

    struct cmd_struct* matched[ARRAY_SIZE(commands)];
    int matched_count = 0;

    for (int i = 0; i < ARRAY_SIZE(commands); ++i) {
        struct cmd_struct* p = commands + i;
        if (!strcmp(p->name, cmd) || !strcmp(p->shortcut, cmd)) {
            matched[matched_count++] = p;
        }
    }

    if (0 == matched_count) {
        die("Unknown command: %s", cmd);
    } else if (1 == matched_count) {
        if (matched[0]->fn) {
            matched[0]->fn(argc, argv);
        } else {
            setup_path(exec_path);
            run_external_cmd(*matched, argc, argv);
        }
    } else {
        fprintf(stderr, "Ambiguous command: ");
        for (int i = 0; i < matched_count; ++i) {
            fprintf(stderr, "%s ", matched[i]->name);
        }
        fprintf(stderr, "\n");
        exit(1);
    }

    return 0;
}
