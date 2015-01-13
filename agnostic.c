
#include "agnostic.h"

#include <assert.h>
#include <string.h>
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

static bool file_exist(const char* fname) {
    return access(fname, F_OK) != -1;
}

static char * normalize_path(const char * src, size_t src_len) {
    // initial version of this function is written by the user 'arnaud576875' from StackOverflow:
    // http://stackoverflow.com/questions/4774116/c-realpath-without-resolving-symlinks        

    char * res;
    size_t res_len;

    const char * ptr = src;
    const char * end = &src[src_len];
    const char * next;

    if (src_len == 0 || src[0] != '/') {
        // relative path
        char* pwd = NULL;
        size_t pwd_len;

        if (!(pwd = getcwd(NULL, 0))) {
            return NULL;
        }

        pwd_len = strlen(pwd);
        res = malloc(pwd_len + 1 + src_len + 1);
        if (!res) {
            free(pwd);
            return NULL;
        }
        memcpy(res, pwd, pwd_len);
        res_len = pwd_len;
        free(pwd);
    } else {
        res = malloc((src_len > 0 ? src_len : 1) + 1);
        if (!res) {
            return NULL;
        }
        res_len = 0;
    }

    for (ptr = src; ptr < end; ptr=next+1) {
        size_t len;
        next = memchr(ptr, '/', end-ptr);
        if (next == NULL) {
            next = end;
        }
        len = next-ptr;
        switch(len) {
        case 2:
            if (ptr[0] == '.' && ptr[1] == '.') {
                const char * slash = strrchr(res, '/');
                if (slash != NULL) {
                    res_len = slash - res;
                }
                continue;
            }
            break;
        case 1:
            if (ptr[0] == '.') {
                continue;
            }
            break;
        case 0:
            continue;
        }
        res[res_len++] = '/';
        memcpy(&res[res_len], ptr, len);
        res_len += len;
    }

    if (res_len == 0) {
        res[res_len++] = '/';
    }
    res[res_len] = '\0';
    return res;
}

char* ag_find_project_file() {
    const int size = 2;
    const char* files[size] = { "../agnostic.yaml", "agnostic.yaml" };
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
    return normalize_path(relative, strlen(relative));  // realpath() doesn't work here, since we DON'T want to resolve symlinks
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

