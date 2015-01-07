
#include "agnostic.h"
#include "common.h"
#include <stdio.h>
#include <stdlib.h>

void component(int argc, const char** argv) {
    struct ag_project* project = NULL;
    if (ag_load_default(&project)) {
        die("Failed to load project");
    }
    struct ag_component* c = ag_find_current_component(project);
    if (c) {
        printf("Name: %s\n", c->name);
        if (c->alias) {
            printf("Alias: %s\n", c->alias);
        }
        if (c->git) {
            printf("Repository: %s (git)\n", c->git);
        } else if (c->hg) {
            printf("Repository: %s (mercurial)\n", c->hg);
        }
        if (c->description && c->description[0]) {
            printf("Description: %s\n", c->description);
        }
        if (c->build_after) {
            struct ag_string_list* l = c->build_after;
            printf("\nBuild after:\n");
            while (l) {
                printf("%s ", l->s);
                l = l->next;
            }
            printf("\n");
        }
        if (c->build) {
            printf("\nBuild:\n%s\n", c->build);
            if (c->integrate) {
                printf("Integration build:\n%s\n", c->integrate);
            }
        }
    } else {
        die("Component not found");
    }
    ag_free(project);
}
