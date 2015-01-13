
#include "agnostic.h"
#include "common.h"
#include <stdio.h>
#include <stdlib.h>

#define PROP_COLOR TERM_COLOR_GREEN

void component(int argc, const char** argv) {
    struct ag_project* project = ag_load_default_or_die();
    struct ag_component* c = ag_find_current_component(project);
    if (c) {
        printf(PROP_COLOR "Name:" TERM_COLOR_RESET " %s\n", c->name);
        if (c->alias) {
            printf(PROP_COLOR "Alias:" TERM_COLOR_RESET " %s\n", c->alias);
        }
        if (c->git) {
            printf(PROP_COLOR "Repository:" TERM_COLOR_RESET " %s (git)\n", c->git);
        } else if (c->hg) {
            printf(PROP_COLOR "Repository:" TERM_COLOR_RESET " %s (mercurial)\n", c->hg);
        }
        if (c->description && c->description[0]) {
            printf(PROP_COLOR "Description:" TERM_COLOR_RESET " %s\n", c->description);
        }
        if (c->build_after) {
            struct ag_string_list* l = c->build_after;
            printf(PROP_COLOR "\nBuild after:\n" TERM_COLOR_RESET);
            while (l) {
                printf("%s ", l->s);
                l = l->next;
            }
            printf("\n");
        }
        if (c->build) {
            printf(PROP_COLOR "\nBuild:" TERM_COLOR_RESET "\n%s\n", c->build);
            if (c->integrate) {
                printf(PROP_COLOR "Integration build:" TERM_COLOR_RESET "\n%s\n", c->integrate);
            }
        }
    } else {
        die("Component not found");
    }
    ag_free(project);
}
