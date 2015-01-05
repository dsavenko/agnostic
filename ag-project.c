
#include "agnostic.h"
#include "common.h"
#include <stdio.h>
#include <stdlib.h>

void project(int argc, const char** argv) {
    char* f = ag_find_project_file();
    if (!f) {
        die("Project file not found");
    }
    printf("%s\n", f);
    free(f);
}
