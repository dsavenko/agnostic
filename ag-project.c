
#include "agnostic.h"
#include "common.h"
#include <stdio.h>
#include <stdlib.h>

void project(int argc, const char** argv) {
    struct ag_project* p = NULL;
    if (ag_load_default(&p)) {
        die("Failed to load project");
    }
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
    ag_free(p);
}
