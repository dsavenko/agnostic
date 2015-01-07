
#include "agnostic.h"

#include <yaml.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

struct ag_component_list* ag_create_component_node(struct ag_component* component, struct ag_component_list* next) {
    if (!component) {
        return NULL;
    }
    struct ag_component_list* ret = (struct ag_component_list*)calloc(1, sizeof(struct ag_component_list));
    if (!ret) {
        ag_shallow_free_component_list(next);
        return NULL;
    }
    ret->component = component;
    ret->next = next;
    return ret;
}

struct ag_string_list* ag_create_string_node(char* s, struct ag_string_list* next) {
    if (!s) {
        return NULL;
    }
    struct ag_string_list* ret = (struct ag_string_list*)calloc(1, sizeof(struct ag_string_list));
    if (!ret) {
        ag_free_string_list(next);
        return NULL;
    }
    ret->s = s;
    ret->next = next;
    return ret;
}

static void ag_free_component(struct ag_component* c);

static void ag_free_component_list(struct ag_component_list* c, bool free_components) {
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

void ag_shallow_free_component_list(struct ag_component_list* c) {
    ag_free_component_list(c, false);
}

void ag_free_string_list(struct ag_string_list* l) {
    struct ag_string_list* n = NULL;
    while (l) {
        free(l->s);
        n = l->next;
        free(l);
        l = n;
    }
}

static void ag_free_component(struct ag_component* c) {
    if (!c) {
        return;
    }
    free(c->name);
    free(c->alias);
    free(c->description);
    free(c->git);
    free(c->hg);
    free(c->build);
    free(c->integrate);
    ag_free_string_list(c->build_after);
    free(c);
}

void ag_free(struct ag_project* p) {
    if (!p) {
        return;
    }
    free(p->name);
    free(p->description);
    free(p->bugs);
    free(p->dir);
    free(p->file);
    ag_free_component_list(p->components, true);
    ag_free_string_list(p->docs);
    free(p);
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

static char* parent_dir(char* absolute_path) {
    char* t = strrchr(absolute_path, '/');
    *t = '\0';
    char* ret = strdup(absolute_path);
    *t = '/';
    if (!ret) {
        return NULL;
    }
    if (!ret[0]) {
        free(ret);
        ret = strdup("/");
    }
    return ret;
}

int ag_load(const char* file_name, struct ag_project** project) {
    assert(file_name);

    FILE *fh = fopen(file_name, "r");
    if (!fh) {
        return 1;
    }

    *project = (struct ag_project*)calloc(1, sizeof(struct ag_project));
    if ('/' == file_name[0]) {
        (*project)->file = strdup(file_name);
    } else {
        (*project)->file = realpath(file_name, NULL);
    }
    if (!(*project)->file) {
        free(*project);
        return 3;
    }

    (*project)->dir = parent_dir((*project)->file);
    if (!(*project)->dir) {
        free((*project)->file);
        free(*project);
        return 4;
    }

    yaml_parser_t parser;
    yaml_token_t token;

    if (!yaml_parser_initialize(&parser)) {
        return 2;
    }

    yaml_parser_set_input_file(&parser, fh);

    struct ag_component_list** c = &((*project)->components);

    enum load_state {
        unknown,
        name, 
        alias, 
        description,
        git,
        hg,
        build,
        integrate,
        build_after,
        bugs,
        docs
    } state = unknown;

    bool is_key = false;
    bool is_project = false;
    bool eof = false;

    while (!eof) {
        yaml_parser_scan(&parser, &token);
        switch(token.type) {
            case YAML_STREAM_END_TOKEN:
                eof = true;
                break;

            case YAML_KEY_TOKEN:   
                is_key = true;
                state = unknown;
                break;

            case YAML_VALUE_TOKEN: 
                is_key = false;
                break;

            case YAML_SCALAR_TOKEN:  
                if (is_key) {
                    const char* key = (const char *)token.data.scalar.value;

                    if (!strcmp(key, "project")) {
                        is_project = true;
                    } else if (!strcmp(key, "component")) {
                        is_project = false;
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

                    } else if (!strcmp(key, "integrate")) {
                        state = integrate;

                    } else if (!strcmp(key, "buildAfter")) {
                        state = build_after;

                    } else if (!strcmp(key, "bugs")) {
                        state = bugs;

                    } else if (!strcmp(key, "docs")) {
                        state = docs;

                    } else {
                        state = unknown;
                    }

                } else {

                    switch (state) {
                    case name:
                        if (is_project) {
                            (*project)->name = strdup((const char*)token.data.scalar.value);
                        } else {
                            (*c)->component->name = strdup((const char*)token.data.scalar.value);
                        }
                        break;

                    case alias:
                        if (!is_project) {
                            (*c)->component->alias = strdup((const char*)token.data.scalar.value);
                        }
                        break;

                    case description:
                        if (is_project) {
                            (*project)->description = strdup((const char*)token.data.scalar.value);
                        } else {
                            (*c)->component->description = strdup((const char*)token.data.scalar.value);
                        }
                        break;
                        
                    case git:
                        if (!is_project) {
                            (*c)->component->git = strdup((const char*)token.data.scalar.value);
                        }
                        break;
                        
                    case hg:
                        if (!is_project) {
                            (*c)->component->hg = strdup((const char*)token.data.scalar.value);
                        }
                        break;
                        
                    case build:
                        if (!is_project) {
                            (*c)->component->build = strdup((const char*)token.data.scalar.value);
                        }
                        break;

                    case integrate:
                        if (!is_project) {
                            (*c)->component->integrate = strdup((const char*)token.data.scalar.value);
                        }
                        break;

                    case build_after:
                        if (!is_project) {
                            (*c)->component->build_after = ag_create_string_node(strdup((const char*)token.data.scalar.value), (*c)->component->build_after);
                        }
                        break;

                    case bugs:
                        if (is_project) {
                            (*project)->bugs = strdup((const char*)token.data.scalar.value);
                        }
                        break;

                    case docs:
                        if (is_project) {
                            (*project)->docs = ag_create_string_node(strdup((const char*)token.data.scalar.value), (*project)->docs);
                        }
                        break;

                    case unknown:
                        break;
                    }
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
    assert(project);

    char* buf = getcwd(NULL, 0);
    if (!buf) {
        return NULL;
    }
    char* name = strrchr(buf, '/');
    name = name ? (name + 1) : buf;
    struct ag_component* ret = ag_find_component(project, name);
    free(buf);
    return ret;
}

struct ag_component* ag_find_component(struct ag_project* project, const char* name_or_alias) {
    assert(project);
    assert(name_or_alias);

    struct ag_component_list* l = project->components;
    struct ag_component* ret = NULL;
    while (l && !ret) {
        if ((l->component->name && !strcmp(l->component->name, name_or_alias)) || 
            (l->component->alias && !strcmp(l->component->alias, name_or_alias))) {
            ret = l->component;
        }
        l = l->next;
    }
    return ret;
}

char* ag_component_dir(struct ag_project* project, struct ag_component* component) {
    assert(project);
    assert(project->dir);
    assert(component);
    assert(component->name);

    char* ret = NULL;
    asprintf(&ret, "%s/%s", project->dir, component->name);
    return ret;
}

static struct ag_component_list* fill_build_up_list(struct ag_component_list* old_root, struct ag_project* project, struct ag_component* component) {
    if (!old_root) {
        return NULL;
    }

    struct ag_component_list* new_root = old_root;
    
    for (struct ag_string_list* slist = component->build_after; slist; slist = slist->next) {
        const char* s = slist->s;

        bool found = false;
        for (struct ag_component_list* l = project->components; l && !found; l = l->next) {
            if (!strcmp(l->component->name, s)) {
                new_root = ag_create_component_node(l->component, new_root);
                new_root = fill_build_up_list(new_root, project, l->component);
                found = true;
            }
        }

        if (!found || !new_root) {
            ag_shallow_free_component_list(new_root);
            return NULL;
        }
    }

    return new_root;
}

static void remove_duplicates(struct ag_component_list* list) {
    for (struct ag_component_list* i = list; i; i = i->next) {
        for (struct ag_component_list* j = i->next, *prev_j = i; j; prev_j = j, j = j->next) {
            if (!strcmp(i->component->name, j->component->name)) {
                prev_j->next = j->next;
                free(j);
                j = prev_j;
            }
        }
    }
}

struct ag_component_list* ag_build_up_list(struct ag_project* project, struct ag_component* component) {
    assert(project);
    assert(component);
    struct ag_component_list* ret = fill_build_up_list(ag_create_component_node(component, NULL), project, component);
    remove_duplicates(ret);
    return ret;
}

