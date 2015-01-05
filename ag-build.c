
#include "agnostic.h"
#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>

void build(int argc, const char** argv) {
    struct ag_project* project = NULL;
    if (ag_load_default(&project)) {
        die("Failed to load project");
    }

    struct ag_component* c = NULL;
    if (1 <= argc) {
        c = ag_find_component(project, *argv);
    } else {
        c = ag_find_current_component(project);
    }

    if (!c) {
        die("Component not found");
    }
    if (!c->build || !c->build[0]) {
        die("Nothing to build");
    }

    char* script = create_temp_file("agnostic-script-", c->build);
    if (!script) {
        die("Unable to create build script.");
    }
    char* parent_dir = ag_component_dir(project, c);
    if (!parent_dir) {
        die("Unable to find parent directory of the component.");
    }
    pid_t child_pid = run_script(parent_dir, script);
    if (-1 == child_pid) {
        perror(NULL);
        die("Failed to run build");
    }
    int status = 0;
    wait(&status);

    remove(script);
    free(script);
    free(parent_dir);
    ag_free(project);    
}
