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
    int component_count;
    struct ag_component_list* components;
};

// Creates a new project. If project_list points to another project, the pointer will be lost.
// Return 0 on success, or error code. 
int ag_load(const char* file_name, struct ag_project** project);

// Frees the whole project structure.
void ag_free(struct ag_project* project);

// Searches for the project file. 
// Returns a newly created absolute path for the project file, or NULL, if not found.
char* ag_create_project_file_name();

#endif /* AGNOSTIC_H */
