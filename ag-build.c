
#include "agnostic.h"
#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>

void build() {
    struct ag_project* project = NULL;
    if (ag_load_default(&project)) {
        die("Failed to load project");
    }
    struct ag_component* c = ag_find_current_component(project);
    if (!c) {
        die("Component not found");
    }
    if (!c->build || !c->build[0]) {
        die("Nothing to build");
    }

    char* script = create_temp_file("agnostic-script-", c->build);
    pid_t child_pid = run_script(script);
    if (-1 == child_pid) {
        perror(NULL);
        die("Failed to run build");
    }
    int status = 0;
    wait(&status);

    remove(script);
    free(script);
    ag_free(project);    
}
