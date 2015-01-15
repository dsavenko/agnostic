
#include "agnostic.h"
#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>
#include <assert.h>

static int dry_run = 0;

static struct ag_component* extract_component(struct ag_project* project, int argc, const char** argv) {
    struct ag_component* ret = NULL;
    if (1 == argc) {
        ret = ag_find_component(project, *argv);
    } else if (2 == argc) {
        if (!strcmp("-c", *argv)) {
            ret = ag_find_component(project, *(argv+1));
        } else {
            die("Unrecognized argument: %s", *argv);
        }
    } else if (0 == argc) {
        ret = ag_find_current_component(project);
    } else {
        die("Too many arguments");
    }
    if (!ret) {
        die("Component not found");
    }
    return ret;
}

static void build_component(struct ag_project* project, struct ag_component* c) {
    assert(project);
    assert(c);

    if (dry_run) {
        printf("%s\n", c->name);
        return;
    }

    printf("Building %s\n", c->name);

    if (!c->build || !c->build[0]) {
        die("Nothing to build");
    }

    char* script = create_temp_file("agnostic-script-", c->build);
    if (!script) {
        die("Unable to create build script.");
    }
    char* parent_dir = ag_component_dir(project, c);
    if (!parent_dir) {
        remove(script);
        die("Unable to find parent directory of the component.");
    }
    debug_print("Running script %s from parent directory %s\n", script, parent_dir);
    pid_t child_pid = run_script(parent_dir, script);
    if (-1 == child_pid) {
        perror(NULL);
        remove(script);
        die("Failed to run build");
    }

    int status = 0;
    wait(&status);

    remove(script);

    if (WIFEXITED(status)) {
        if (WEXITSTATUS(status)) {
            die("Failed to build %s", c->name);
        }
    } else {
        die("Build interrupted: %s", c->name);
    }

    free(script);
    free(parent_dir);
}

static void build_current(struct ag_project* project) {
    build_component(project, extract_component(project, 0, NULL));
}

static void build_list(struct ag_project* project, int argc, const char** argv) {
    while (0 < argc) {
        struct ag_component* c = ag_find_component(project, *argv);
        // TODO: re-write this to support -c and use extract_component()
        if (!c) {
            die("Component not found: %s", *argv);
        }
        --argc;
        ++argv;
        build_component(project, c);
    }
}

static void build_up(struct ag_project* project, int argc, const char** argv) {
    const char* up_to = NULL;

    while (1 <= argc) {
        if (!strcmp("-t", *argv) || !strcmp("--to", *argv)) {
            if (2 > argc) {
                die("Expected component name/alias after %s", *argv);
            }
            ++argv;
            --argc;
            up_to = *argv;
        } else {
            break;
        }
        ++argv;
        --argc;
    }

    struct ag_component_list* deps = ag_build_up_list(project, extract_component(project, argc, argv), up_to);
    if (!deps) {
        die("Unable to resolve build order");
    }
    for (struct ag_component_list* l = deps; l; l = l->next) {
        build_component(project, l->component);
    }
    ag_shallow_free_component_list(deps);
}

static void build_down(struct ag_project* project, int argc, const char** argv) {
    const char* down_to = NULL;

    while (1 <= argc) {
        if (!strcmp("-t", *argv) || !strcmp("--to", *argv)) {
            if (2 > argc) {
                die("Expected component name/alias after %s", *argv);
            }
            ++argv;
            --argc;
            down_to = *argv;
        } else {
            break;
        }
        ++argv;
        --argc;
    }

    struct ag_component_list* deps = ag_build_down_list(project, extract_component(project, argc, argv), down_to);
    if (!deps) {
        die("Unable to resolve build order");
    }
    for (struct ag_component_list* l = deps; l; l = l->next) {
        build_component(project, l->component);
    }
    ag_shallow_free_component_list(deps);
}

void build(int argc, const char** argv) {
    struct ag_project* project = ag_load_default_or_die();

    // options
    while (1 <= argc) {
        if (!strcmp("-n", *argv) || !strcmp("--dry-run", *argv)) {
            dry_run = 1;
        } else {
            break;
        }
        --argc;
        ++argv;
    }

    // command
    if (1 <= argc) {
        if (!strcmp("up", *argv)) {
            build_up(project, argc-1, argv+1);

        } else if (!strcmp("down", *argv)) {
            build_down(project, argc-1, argv+1);
            
        } else {
            build_list(project, argc, argv);
        }
    } else {
        build_current(project);
    }
    ag_free(project);    
}
