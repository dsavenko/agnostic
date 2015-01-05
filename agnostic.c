
#include "agnostic.h"

#include <yaml.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

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
    free(c->name);
    free(c->alias);
    free(c->description);
    free(c->git);
    free(c->hg);
    free(c->build);
    ag_free_component_list(c->build_after, false);
}

void ag_free(struct ag_project* project) {
    if (!project) {
        return;
    }
    ag_free_component_list(project->components, true);
    free(project);
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
                        (*project)->component_count++;

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
                        (*c)->component->name = strdup((const char*)token.data.scalar.value);
                        break;

                    case alias:
                        (*c)->component->alias = strdup((const char*)token.data.scalar.value);
                        break;

                    case description:
                        (*c)->component->description = strdup((const char*)token.data.scalar.value);
                        break;
                        
                    case git:
                        (*c)->component->git = strdup((const char*)token.data.scalar.value);
                        break;
                        
                    case hg:
                        (*c)->component->hg = strdup((const char*)token.data.scalar.value);
                        break;
                        
                    case build:
                        (*c)->component->build = strdup((const char*)token.data.scalar.value);
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

static bool file_exist(const char* fname) {
    return access(fname, F_OK) != -1;
}

char* ag_find_project_file() {
    const int size = 2;
    const char* files[size] = { "agnostic.yaml", "../agnostic.yaml" };
    const char* relative = NULL;
    for (int i = 0; i < 2; ++i) {
        if (file_exist(files[i])) {
            relative = files[i];
            break;
        }
    }
    if (!relative) {
        return NULL;
    }
    return realpath(relative, NULL);
}

struct ag_component* ag_find_current_component(struct ag_project* project) {
    char* buf = getcwd(NULL, 0);
    if (!buf) {
        return NULL;
    }
    char* name = strrchr(buf, '/');
    name = name ? (name + 1) : buf;
    struct ag_component_list* l = project->components;
    struct ag_component* ret = NULL;
    while (l && !ret) {
        if (!strcmp(l->component->name, name)) {
            ret = l->component;
        }
        l = l->next;
    }
    free(buf);
    return ret;
}

