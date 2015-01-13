
#include "agnostic.h"
#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void print_all(struct ag_project* p) {
    printf("Name: %s\n", p->name);
    printf("Root directory: %s\n", p->dir);
    printf("Project file: %s\n", p->file);
    if (p->description && p->description[0]) {
        printf("Description: %s\n", p->description);
    }
    if (p->bugs) {
        printf("Bug tracker: %s\n", p->bugs);
    }
    if (p->docs) {
        printf("Documentation:\n");
        for (struct ag_string_list* l = p->docs; l; l = l->next) {
            printf("  - %s\n", l->s);
        }
    }
    if (p->components) {
        printf("Components (%d):\n", p->component_count);
        for (struct ag_component_list* l = p->components; l; l = l->next) {
            printf("  - %s\n", l->component->name);
        }
    }
}

static void print_directories(struct ag_project* p) {
    for (struct ag_component_list* l = p->components; l; l = l->next) {
        printf("%s\n", l->component->name);
        if (l->component->alias) {
            printf("%s\n", l->component->alias);
        }
    }
}

void project(int argc, const char** argv) {
    struct ag_project* p = ag_load_default_or_die();
    if (0 == argc) {
        print_all(p);
    } else if (1 == argc) {
        if (!strcmp("directories", *argv) || !strcmp("dirs", *argv)) {
            print_directories(p);
        } else {
            die("Unknown argument: %s", *argv);
        }
    } else {
        die("Too many arguments");
    }
    ag_free(p);
}
