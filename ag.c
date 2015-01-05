
#include "agnostic.h"
#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
        { "clone", "", &clone },
        { "component", "comp", &component },
        { "project", "proj", &project },
        { "build", "", &build },
        { "help", "", &help }
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

int main(int argc, char **av) {
    const char **argv = (const char **) av;

    if (1 >= argc) {
        help(0, NULL);
        return 0;
    }

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
        matched[0]->fn(argc, argv);
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
