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
    char* integrate;
    struct ag_string_list* build_after;
};

struct ag_component_list {
    struct ag_component* component;
    struct ag_component_list* next;
};

struct ag_project {
    char* name;
    char* description;
    char* bugs;
    char* dir;
    char* file;
    int component_count;
    struct ag_component_list* components;
    struct ag_string_list* docs;
};

enum ag_return_codes {
    OK,
    UNABLE_TO_OPEN_FILE,
    FILE_NOT_FOUND,
    PROJECT_GOES_AFTER_COMPONENT
};

// Returns error message for the given code, or NULL, if not found.
// Error message must not be freed.
const char* ag_error_msg(int code);

// Creates a new string node. Does nothing and return NULL, if string is NULL or failed to allocate memory. 
// If failed to allocate memory, and next is not NULL, frees next. 
struct ag_string_list* ag_create_string_node(char* s, struct ag_string_list* next);

// Frees the given string list with all its strings.
void ag_free_string_list(struct ag_string_list* l);

// Creates a new component node. Does nothing and return NULL, if the given component is NULL or failed to allocate memory. 
// If failed to allocate memory, and next is not NULL, frees next. 
struct ag_component_list* ag_create_component_node(struct ag_component* component, struct ag_component_list* next);

// Creates a new project. If project points to another project, the pointer will be lost.
// Return 0 on success, or error code. 
int ag_load(const char* file_name, struct ag_project** project);

// Tries to find default project file and load project from it.
int ag_load_default(struct ag_project** project);

// Tries to find default project file and load project from it. If failed, calls die() with appropriate message.
// On success, returns a newly created project, which should be freed by calling ag_free().
struct ag_project* ag_load_default_or_die();

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
// If 'up_to_component' is not NULL, the list will be built up to this component only.
// The list should be freed with ag_shallow_free_component_list().
struct ag_component_list* ag_build_up_list(struct ag_project* project, struct ag_component* component, const char* up_to_component);

// Returns a list of components, which should be built after the given component. 
// On success, the list always includes the given component as its first item. On failure to resolve dependencies, NULL is returned.
// Components in the list are sorted appropriately.
// If 'down_to_component' is not NULL, the list will be built down to this component only.
// The list should be freed with ag_shallow_free_component_list().
struct ag_component_list* ag_build_down_list(struct ag_project* project, struct ag_component* component, const char* down_to_component);

#endif /* AGNOSTIC_H */
