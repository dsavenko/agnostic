
#include "agnostic.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void print_all(struct ag_project* p) {
    printf(PROP_COLOR "Name:" TERM_COLOR_RESET " %s\n", p->name);
    printf(PROP_COLOR "Root directory:" TERM_COLOR_RESET " %s\n", p->dir);
    printf(PROP_COLOR "Project file:" TERM_COLOR_RESET " %s\n", p->file);
    if (p->description && p->description[0]) {
        printf(PROP_COLOR "Description:" TERM_COLOR_RESET " %s\n", p->description);
    }
    if (p->bugs) {
        printf(PROP_COLOR "Bug tracker:" TERM_COLOR_RESET " %s\n", p->bugs);
    }
    if (p->docs) {
        printf(PROP_COLOR "Documentation:\n" TERM_COLOR_RESET);
        for (struct list* l = p->docs; l; l = l->next) {
            printf("  - %s\n", l->data);
        }
    }
    if (p->components) {
        printf(PROP_COLOR "Components (%d):\n" TERM_COLOR_RESET, p->component_count);
        for (struct list* l = p->components; l; l = l->next) {
            struct ag_component* c = (struct ag_component*)l->data;
            printf("  - %s\n", c->name);
        }
    }
}

static void print_directories(struct ag_project* p) {
    for (struct list* l = p->components; l; l = l->next) {
        struct ag_component* c = (struct ag_component*)l->data;
        printf("%s/%s\n", p->dir, c->name);
        if (c->alias) {
            printf("%s/%s\n", p->dir, c->alias);
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
