
#include "agnostic.h"

#include <yaml.h>
#include <stdio.h>

void ag_free_component(struct ag_component* c);

void ag_free_component_list(struct ag_component_list* c, bool free_components) {
    struct ag_component_list* n = NULL;
    while (c) {
        if (free_components) {
            ag_free_component(c->component);
        }
        n = c->next;
        free(c);
        c = n;
    }
}

void ag_free_component(struct ag_component* c) {
    if (!c) {
        return;
    }
    sdsfree(c->name);
    sdsfree(c->alias);
    sdsfree(c->description);
    sdsfree(c->git);
    sdsfree(c->hg);
    sdsfree(c->build);
    ag_free_component_list(c->build_after, false);
}

void ag_free(struct ag_project* project) {
    if (!project) {
        return;
    }
    ag_free_component_list(project->components, true);
    free(project);
}

int ag_load(const char* file_name, struct ag_project** project) {
    FILE *fh = fopen(file_name, "r");
    if (!fh) {
        return 1;
    }

    yaml_parser_t parser;
    yaml_token_t token;

    if (!yaml_parser_initialize(&parser)) {
        return 2;
    }

    yaml_parser_set_input_file(&parser, fh);

    bool is_key = false;

    *project = (struct ag_project*)calloc(1, sizeof(struct ag_project));
    struct ag_component_list** c = &((*project)->components);

    enum load_state {
        unknown,
        name, 
        alias, 
        description,
        git,
        hg,
        build
    } state = unknown;

    bool eof = false;

    while (!eof) {
        yaml_parser_scan(&parser, &token);
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

            case YAML_SCALAR_TOKEN:  
                if (is_key) {
                    const char* key = (const char *)token.data.scalar.value;

                    if (!strcmp(key, "component")) {
                        if (*c) {
                            (*c)->next = (struct ag_component_list*)calloc(1, sizeof(struct ag_component_list));
                            c = &((*c)->next);
                        } else {
                            *c = (struct ag_component_list*)calloc(1, sizeof(struct ag_component_list));
                        }
                        (*c)->component = (struct ag_component*)calloc(1, sizeof(struct ag_component));

                    } else if (!strcmp(key, "name")) {
                        state = name;

                    } else if (!strcmp(key, "alias")) {
                        state = alias;

                    } else if (!strcmp(key, "description")) {
                        state = description;

                    } else if (!strcmp(key, "git")) {
                        state = git;

                    } else if (!strcmp(key, "hg")) {
                        state = hg;

                    } else if (!strcmp(key, "build")) {
                        state = build;

                    } else {
                        state = unknown;
                    }

                } else {

                    switch (state) {
                    case name:
                        (*c)->component->name = sdsnew((const char*)token.data.scalar.value);
                        break;

                    case alias:
                        (*c)->component->alias = sdsnew((const char*)token.data.scalar.value);
                        break;

                    case description:
                        (*c)->component->description = sdsnew((const char*)token.data.scalar.value);
                        break;
                        
                    case git:
                        (*c)->component->git = sdsnew((const char*)token.data.scalar.value);
                        break;
                        
                    case hg:
                        (*c)->component->hg = sdsnew((const char*)token.data.scalar.value);
                        break;
                        
                    case build:
                        (*c)->component->build = sdsnew((const char*)token.data.scalar.value);
                        break;

                    case unknown:
                        break;
                    }

                    state = unknown;
                }
                break;

            default:
                break;
        }
        
        yaml_token_delete(&token);
    }

    yaml_parser_delete(&parser);
    fclose(fh);
    return 0;
}

