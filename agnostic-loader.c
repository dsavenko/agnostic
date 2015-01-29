
#include "agnostic.h"

#include <yaml.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

static const char* yaml_token_names[] = {
    /** An empty token. */
    "YAML_NO_TOKEN",

    /** A STREAM-START token. */
    "YAML_STREAM_START_TOKEN",
    /** A STREAM-END token. */
    "YAML_STREAM_END_TOKEN",

    /** A VERSION-DIRECTIVE token. */
    "YAML_VERSION_DIRECTIVE_TOKEN",
    /** A TAG-DIRECTIVE token. */
    "YAML_TAG_DIRECTIVE_TOKEN",
    /** A DOCUMENT-START token. */
    "YAML_DOCUMENT_START_TOKEN",
    /** A DOCUMENT-END token. */
    "YAML_DOCUMENT_END_TOKEN",

    /** A BLOCK-SEQUENCE-START token. */
    "YAML_BLOCK_SEQUENCE_START_TOKEN",
    /** A BLOCK-SEQUENCE-END token. */
    "YAML_BLOCK_MAPPING_START_TOKEN",
    /** A BLOCK-END token. */
    "YAML_BLOCK_END_TOKEN",

    /** A FLOW-SEQUENCE-START token. */
    "YAML_FLOW_SEQUENCE_START_TOKEN",
    /** A FLOW-SEQUENCE-END token. */
    "YAML_FLOW_SEQUENCE_END_TOKEN",
    /** A FLOW-MAPPING-START token. */
    "YAML_FLOW_MAPPING_START_TOKEN",
    /** A FLOW-MAPPING-END token. */
    "YAML_FLOW_MAPPING_END_TOKEN",

    /** A BLOCK-ENTRY token. */
    "YAML_BLOCK_ENTRY_TOKEN",
    /** A FLOW-ENTRY token. */
    "YAML_FLOW_ENTRY_TOKEN",
    /** A KEY token. */
    "YAML_KEY_TOKEN",
    /** A VALUE token. */
    "YAML_VALUE_TOKEN",

    /** An ALIAS token. */
    "YAML_ALIAS_TOKEN",
    /** An ANCHOR token. */
    "YAML_ANCHOR_TOKEN",
    /** A TAG token. */
    "YAML_TAG_TOKEN",
    /** A SCALAR token. */
    "YAML_SCALAR_TOKEN"
};

enum structure_state {
    s_unknown,
    s_doc_root,
    s_project,
    s_project_docs,
    s_component,
    s_component_build_after,

    __s_length
};

int ag_load_default(struct ag_project** project) {
    char* cfg_file = ag_find_project_file();
    if (!cfg_file) {
        return FILE_NOT_FOUND;
    }
    int ret = ag_load(cfg_file, project);
    free(cfg_file);
    return ret;
}

struct ag_project* ag_load_default_or_die() {
    struct ag_project* ret = NULL;
    int x = ag_load_default(&ret);
    if (x) {
        die("Failed to load the project. %s.", ag_error_msg(x));
    }
    return ret;
}

static int stack_v(struct list* stack) {
    return stack ? *((int*)stack->data) : -1;
}

int ag_load(const char* file_name, struct ag_project** project) {
    assert(file_name);

    FILE *fh = fopen(file_name, "r");
    if (!fh) {
        return UNABLE_TO_OPEN_FILE;
    }

    *project = (struct ag_project*)xcalloc(1, sizeof(struct ag_project));
    if ('/' == file_name[0]) {
        (*project)->file = xstrdup(file_name);
    } else {
        (*project)->file = realpath(file_name, NULL);
    }
    if (!(*project)->file) {
        free(*project);
        return FILE_NOT_FOUND;
    }

    (*project)->dir = parent_dir((*project)->file);

    yaml_parser_t parser;
    yaml_token_t token;

    if (!yaml_parser_initialize(&parser)) {
        die("Unable to initialize YAML parser");
    }
    yaml_parser_set_input_file(&parser, fh);

    struct ag_component* component = NULL;
    
    struct list* components_head = NULL;
    struct list* components_tail = NULL;

    struct list* build_after_head = NULL;
    struct list* build_after_tail = NULL;

    struct list* docs_head = NULL;
    struct list* docs_tail = NULL;

    struct list* stack = NULL;
    int stack_vals[__s_length];
    for (int i = 0; i < __s_length; ++i) {
        stack_vals[i] = i;
    }

    char* key = NULL;

    int is_key = 0;
    int eof = 0;
    int ret = OK;

    while (!eof) {
        yaml_parser_scan(&parser, &token);
        debug_print("token %s\n", yaml_token_names[token.type]);
        int sval = stack_v(stack);
        switch(token.type) {
            case YAML_STREAM_END_TOKEN:
                eof = 1;
                break;

            case YAML_KEY_TOKEN:   
                is_key = 1;
                break;

            case YAML_VALUE_TOKEN: 
                is_key = 0;
                break;

            case YAML_DOCUMENT_START_TOKEN:
                list_free(stack, NULL);
                stack = list_create(stack_vals + s_doc_root, NULL);
                break;

            case YAML_DOCUMENT_END_TOKEN:
                list_free(stack, NULL);
                stack = NULL;
                break;

            case YAML_BLOCK_SEQUENCE_START_TOKEN:
            case YAML_BLOCK_MAPPING_START_TOKEN:
            case YAML_FLOW_SEQUENCE_START_TOKEN:
            case YAML_FLOW_MAPPING_START_TOKEN:
                debug_print("started block or flow with key '%s', stack state is %d\n", key, sval);

                if (!key) {
                    break;
                }
                if (s_doc_root == sval && !strcmp(key, "project")) {
                    if ((*project)->components) {
                        eof = 1;
                        ret = PROJECT_GOES_AFTER_COMPONENT;
                        break;
                    }
                    stack = list_create(stack_vals + s_project, stack);
                    debug_print("%s\n", "push project");

                } else if (s_project == sval && !strcmp(key, "docs")) {
                    stack = list_create(stack_vals + s_project_docs, stack);
                    debug_print("%s\n", "push project docs");

                } else if (s_doc_root == sval && !strcmp(key, "component")) {
                    stack = list_create(stack_vals + s_component, stack);
                    component = (struct ag_component*)xcalloc(1, sizeof(struct ag_component));
                    build_after_head = NULL;
                    build_after_tail = NULL;
                    list_add(&components_head, &components_tail, component);
                    (*project)->component_count++;
                    debug_print("%s\n", "push component");

                } else if (s_component == sval && !strcmp(key, "buildAfter")) {
                    stack = list_create(stack_vals + s_component_build_after, stack);
                    debug_print("%s\n", "push component build after");

                } else {
                    stack = list_create(stack_vals + s_unknown, stack);
                    debug_print("%s\n", "push unknown");
                }
                break;

            case YAML_BLOCK_END_TOKEN:
            case YAML_FLOW_SEQUENCE_END_TOKEN:
            case YAML_FLOW_MAPPING_END_TOKEN:
                if (s_component == sval) {
                    if (component) {
                        component->build_after = build_after_head;
                    }
                }
                list_pop(&stack);
                break;

            case YAML_SCALAR_TOKEN:  
                if (is_key) {
                    free(key);
                    key = xstrdup((const char *)token.data.scalar.value);

                } else {

                    if (s_project == sval) {
                        if (!strcmp(key, "name")) {
                            (*project)->name = xstrdup((const char*)token.data.scalar.value);

                        } else if (!strcmp(key, "description")) {
                            (*project)->description = xstrdup((const char*)token.data.scalar.value);

                        } else if (!strcmp(key, "bugs")) {
                            (*project)->bugs = xstrdup((const char*)token.data.scalar.value);

                        }

                    } else if (s_project_docs == sval) {
                        list_add(&docs_head, &docs_tail, xstrdup((const char*)token.data.scalar.value));

                    } else if (s_component == sval) {
                        if (!strcmp(key, "name")) {
                            component->name = xstrdup((const char*)token.data.scalar.value);

                        } else if (!strcmp(key, "alias")) {
                            component->alias = xstrdup((const char*)token.data.scalar.value);

                        } else if (!strcmp(key, "description")) {
                            component->description = xstrdup((const char*)token.data.scalar.value);

                        } else if (!strcmp(key, "git")) {
                            component->git = xstrdup((const char*)token.data.scalar.value);

                        } else if (!strcmp(key, "hg")) {
                            component->hg = xstrdup((const char*)token.data.scalar.value);

                        } else if (!strcmp(key, "build")) {
                            component->build = xstrdup((const char*)token.data.scalar.value);

                        } else if (!strcmp(key, "integrate")) {
                            component->integrate = xstrdup((const char*)token.data.scalar.value);

                        } else if (!strcmp(key, "clean")) {
                            component->clean = xstrdup((const char*)token.data.scalar.value);

                        } else if (!strcmp(key, "test")) {
                            component->test = xstrdup((const char*)token.data.scalar.value);

                        }

                    } else if (s_component_build_after == sval) {
                        list_add(&build_after_head, &build_after_tail, xstrdup((const char*)token.data.scalar.value));

                    }

                    free(key);
                    key = NULL;
                }
                break;

            default:
                break;
        }
        
        yaml_token_delete(&token);
    }

    (*project)->docs = docs_head;
    (*project)->components = components_head;

    free(key);
    list_free(stack, NULL);
    yaml_parser_delete(&parser);
    fclose(fh);
    return ret;
}
