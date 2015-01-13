
#include "agnostic.h"
#include "common.h"

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
    s_component_build_after
};

struct structure_stack {
    enum structure_state state;
    struct structure_stack* next;
};

static void s_free(struct structure_stack* stack) {
    while (stack) {
        struct structure_stack* n = stack->next;
        free(stack);
        stack = n;
    }
}

static struct structure_stack* s_push(enum structure_state state, struct structure_stack* stack) {
    struct structure_stack* ret = (struct structure_stack*)xcalloc(1, sizeof(struct structure_stack));
    ret->state = state;
    ret->next = stack;
    return ret;
}

static struct structure_stack* s_pop(struct structure_stack* stack) {
    if (!stack) {
        return NULL;
    }
    struct structure_stack* ret = stack->next;
    free(stack);
    return ret;
}

static struct ag_string_list** append_string_node(char* s, struct ag_string_list** prev) {
    assert(!*prev);
    if (!s) {
        return prev;
    }
    struct ag_string_list* ret = (struct ag_string_list*)xcalloc(1, sizeof(struct ag_string_list));
    ret->s = s;
    *prev = ret;
    prev = &(ret->next);
    return prev;
}

int ag_load_default(struct ag_project** project) {
    char* cfg_file = ag_find_project_file();
    if (!cfg_file) {
        return 3;
    }
    int ret = ag_load(cfg_file, project);
    free(cfg_file);
    return ret;
}

struct ag_project* ag_load_default_or_die() {
    struct ag_project* ret = NULL;
    int x = ag_load_default(&ret);
    if (x) {
        die("Failed to load the project. %s", ag_error_msg(x));
    }
    return ret;
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

    struct ag_component_list** c = &((*project)->components);
    struct ag_string_list** cur_build_after = NULL;
    struct ag_string_list** cur_docs = NULL;

    struct structure_stack* stack = NULL;
    char* key = NULL;

    bool is_key = false;
    bool eof = false;
    int ret = OK;

    while (!eof) {
        yaml_parser_scan(&parser, &token);
        debug_print("token %s\n", yaml_token_names[token.type]);
        switch(token.type) {
            case YAML_STREAM_END_TOKEN:
                eof = true;
                break;

            case YAML_KEY_TOKEN:   
                is_key = true;
                break;

            case YAML_VALUE_TOKEN: 
                is_key = false;
                break;

            case YAML_DOCUMENT_START_TOKEN:
                s_free(stack);
                stack = s_push(s_doc_root, NULL);
                break;

            case YAML_DOCUMENT_END_TOKEN:
                s_free(stack);
                stack = NULL;
                break;

            case YAML_BLOCK_SEQUENCE_START_TOKEN:
            case YAML_BLOCK_MAPPING_START_TOKEN:
            case YAML_FLOW_SEQUENCE_START_TOKEN:
            case YAML_FLOW_MAPPING_START_TOKEN:
                debug_print("started block or flow with key '%s', stack state is %d\n", key, stack->state);

                if (!key) {
                    break;
                }
                if (s_doc_root == stack->state && !strcmp(key, "project")) {
                    if ((*project)->components) {
                        eof = true;
                        ret = PROJECT_GOES_AFTER_COMPONENT;
                        break;
                    }
                    stack = s_push(s_project, stack);
                    debug_print("%s\n", "push project");

                } else if (s_project == stack->state && !strcmp(key, "docs")) {
                    stack = s_push(s_project_docs, stack);
                    cur_docs = &((*project)->docs);
                    debug_print("%s\n", "push project docs");

                } else if (s_doc_root == stack->state && !strcmp(key, "component")) {
                    stack = s_push(s_component, stack);
                    if (*c) {
                        (*c)->next = (struct ag_component_list*)xcalloc(1, sizeof(struct ag_component_list));
                        c = &((*c)->next);
                    } else {
                        *c = (struct ag_component_list*)xcalloc(1, sizeof(struct ag_component_list));
                    }
                    (*c)->component = (struct ag_component*)xcalloc(1, sizeof(struct ag_component));
                    (*project)->component_count++;
                    debug_print("%s\n", "push component");

                } else if (s_component == stack->state && !strcmp(key, "buildAfter")) {
                    stack = s_push(s_component_build_after, stack);
                    cur_build_after = &((*c)->component->build_after);
                    debug_print("%s\n", "push component build after");

                } else {
                    stack = s_push(s_unknown, stack);
                    debug_print("%s\n", "push unknown");
                }
                break;

            case YAML_BLOCK_END_TOKEN:
            case YAML_FLOW_SEQUENCE_END_TOKEN:
            case YAML_FLOW_MAPPING_END_TOKEN:
                stack = s_pop(stack);
                break;

            case YAML_SCALAR_TOKEN:  
                if (is_key) {
                    free(key);
                    key = xstrdup((const char *)token.data.scalar.value);

                } else {

                    if (s_project == stack->state) {
                        if (!strcmp(key, "name")) {
                            (*project)->name = xstrdup((const char*)token.data.scalar.value);

                        } else if (!strcmp(key, "description")) {
                            (*project)->description = xstrdup((const char*)token.data.scalar.value);

                        } else if (!strcmp(key, "bugs")) {
                            (*project)->bugs = xstrdup((const char*)token.data.scalar.value);

                        }

                    } else if (s_project_docs == stack->state) {
                        cur_docs = append_string_node(xstrdup((const char*)token.data.scalar.value), cur_docs);

                    } else if (s_component == stack->state) {
                        if (!strcmp(key, "name")) {
                            (*c)->component->name = xstrdup((const char*)token.data.scalar.value);

                        } else if (!strcmp(key, "alias")) {
                            (*c)->component->alias = xstrdup((const char*)token.data.scalar.value);

                        } else if (!strcmp(key, "description")) {
                            (*c)->component->description = xstrdup((const char*)token.data.scalar.value);

                        } else if (!strcmp(key, "git")) {
                            (*c)->component->git = xstrdup((const char*)token.data.scalar.value);

                        } else if (!strcmp(key, "hg")) {
                            (*c)->component->hg = xstrdup((const char*)token.data.scalar.value);

                        } else if (!strcmp(key, "build")) {
                            (*c)->component->build = xstrdup((const char*)token.data.scalar.value);

                        } else if (!strcmp(key, "integrate")) {
                            (*c)->component->integrate = xstrdup((const char*)token.data.scalar.value);

                        }

                    } else if (s_component_build_after == stack->state) {
                        cur_build_after = append_string_node(xstrdup((const char*)token.data.scalar.value), cur_build_after);

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

    free(key);
    s_free(stack);
    yaml_parser_delete(&parser);
    fclose(fh);
    return ret;
}
