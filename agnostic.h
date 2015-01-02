#ifndef AGNOSTIC_H
#define AGNOSTIC_H

#include "sds.h"
#include <stdbool.h>

struct ag_component_list;

struct ag_component {
    sds name;
    sds alias;
    sds description;
    sds git;
    sds hg;
    sds build;
    struct ag_component_list* build_after;
};

struct ag_component_list {
    struct ag_component* component;
    struct ag_component_list* next;
};

struct ag_project {
    struct ag_component_list* components;
};

// Creates a new project. If project_list points to another project, the pointer will be lost.
// Return 0 on success, or error code. 
int ag_load(const char* file_name, struct ag_project** project);

void ag_free(struct ag_project* project);

#endif /* AGNOSTIC_H */
