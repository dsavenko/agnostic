#ifndef AGNOSTIC_H
#define AGNOSTIC_H

#include <stdbool.h>

struct ag_component_list;

struct ag_component {
    char* name;
    char* alias;
    char* description;
    char* git;
    char* hg;
    char* build;
};

struct ag_project {
    char* dir;
    char* file;
    int component_count;
    struct ag_component** components;
};

// Creates a new project. If project points to another project, the pointer will be lost.
// Return 0 on success, or error code. 
int ag_load(const char* file_name, struct ag_project** project);

// Tries to find default project file and load project from it.
int ag_load_default(struct ag_project** project);

// Frees the whole project structure.
void ag_free(struct ag_project* project);

// Searches for the project file. 
// Returns full path to the project file, which may later be freed, or NULL, if not found.
char* ag_find_project_file();

// Returns current component of the given project.
struct ag_component* ag_find_current_component(struct ag_project* project);

// Searches for component by the given name or alias.
struct ag_component* ag_find_component(struct ag_project* project, const char* name_or_alias);

// Returns the given component directory.
char* ag_component_dir(struct ag_project* project, struct ag_component* component);

#endif /* AGNOSTIC_H */
