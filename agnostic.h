#ifndef AGNOSTIC_H
#define AGNOSTIC_H

#include <stdbool.h>

struct ag_string_list {
    char* s;
    struct ag_string_list* next;
};

struct ag_component {
    char* name;
    char* alias;
    char* description;
    char* git;
    char* hg;
    char* build;
    struct ag_string_list* build_after;
};

struct ag_component_list {
    struct ag_component* component;
    struct ag_component_list* next;
};

struct ag_project {
    char* dir;
    char* file;
    int component_count;
    struct ag_component_list* components;
};

// Creates a new component node. Does nothing and return NULL, if the given component is NULL.
struct ag_component_list* ag_create_component_node(struct ag_component* component, struct ag_component_list* next);

// Creates a new project. If project points to another project, the pointer will be lost.
// Return 0 on success, or error code. 
int ag_load(const char* file_name, struct ag_project** project);

// Tries to find default project file and load project from it.
int ag_load_default(struct ag_project** project);

// Frees the whole project structure.
void ag_free(struct ag_project* project);

// Frees the given component list, but does not free components themselves.
void ag_shallow_free_component_list(struct ag_component_list* c);

// Searches for the project file. 
// Returns full path to the project file, which may later be freed, or NULL, if not found.
char* ag_find_project_file();

// Returns current component of the given project.
struct ag_component* ag_find_current_component(struct ag_project* project);

// Searches for component by the given name or alias.
struct ag_component* ag_find_component(struct ag_project* project, const char* name_or_alias);

// Returns the given component directory.
char* ag_component_dir(struct ag_project* project, struct ag_component* component);

// Returns a list of components, which should be built before the given component. 
// On success, the list always includes the given component as its last item. On failure to resolve dependencies, NULL is returned.
// Components in the list are sorted appropriately. 
// The list should be freed with ag_shallow_free_component_list().
struct ag_component_list* ag_build_up_list(struct ag_project* project, struct ag_component* component);

#endif /* AGNOSTIC_H */
