
#include "agnostic.h"
#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


struct cmd_struct {
    const char* name;
    void (*fn)();
};

extern void clone();
extern void component();

static void help();

static void project_file() {
    char* f = ag_find_project_file();
    if (!f) {
        die("Project file not found");
    }
    printf("%s\n", f);
    free(f);
}

static struct cmd_struct commands[] = {
        { "clone", &clone },
        { "component", &component },
        { "project", &project_file },
        { "help", &help }
    };

static void help() {
    printf("%s\n%s", 
        "ag-info <command>", 
        "Recognized commands: ");
    for (int i = 0; i < ARRAY_SIZE(commands); ++i) {
        printf("%s ", (commands + i)->name);
    }
    printf("\n");
}

static bool starts_with(const char *pre, const char *str) {
    size_t lenpre = strlen(pre);
    size_t lenstr = strlen(str);
    return lenstr < lenpre ? false : strncmp(pre, str, lenpre) == 0;
}

int main(int argc, char **av) {
    const char **argv = (const char **) av;

    if (1 >= argc) {
        help();
        return 0;
    }

    ++argv;
    const char *cmd = *argv;

    struct cmd_struct* matched[ARRAY_SIZE(commands)];
    int matched_count = 0;

    for (int i = 0; i < ARRAY_SIZE(commands); ++i) {
        struct cmd_struct* p = commands + i;
        if (starts_with(cmd, p->name)) {
            matched[matched_count++] = p;
        }
    }

    if (0 == matched_count) {
        die("Unknown command: %s", cmd);
    } else if (1 == matched_count) {
        matched[0]->fn();
    } else {
        fprintf(stderr, "Ambiguous shortening: ");
        for (int i = 0; i < matched_count; ++i) {
            fprintf(stderr, "%s ", matched[i]->name);
        }
        fprintf(stderr, "\n");
        exit(1);
    }

    return 0;
}
