
#include "agnostic.h"

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static const char* error_messages[] = {
    [OK] = "OK",
    [UNABLE_TO_OPEN_FILE] = "Unable to open file",
    [FILE_NOT_FOUND] = "File not found",
    [PROJECT_GOES_AFTER_COMPONENT] = "Project section must go before any component sections"
};

const char* ag_error_msg(int code) {
    if (0 > code || code >= ARRAY_SIZE(error_messages)) {
        return NULL;
    }
    return error_messages[code];
}

static void ag_free_component(void* data) {
    if (!data) {
        return;
    }
    struct ag_component* c = (struct ag_component*)data;
    free(c->name);
    free(c->alias);
    free(c->description);
    free(c->git);
    free(c->hg);
    free(c->build);
    free(c->integrate);
    free(c->clean);
    list_free(c->build_after, &free);
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
    list_free(p->components, &ag_free_component);
    list_free(p->docs, &free);
    free(p);
}

static int file_exist(const char* fname) {
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
        res = xmalloc(pwd_len + 1 + src_len + 1);
        memcpy(res, pwd, pwd_len);
        res_len = pwd_len;
        free(pwd);
    } else {
        res = xmalloc((src_len > 0 ? src_len : 1) + 1);
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
    if (!strcmp(project->dir, buf)) {
        // we're in the project directory -> no current component.
        free(buf);
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

    struct list* l = project->components;
    struct ag_component* ret = NULL;
    while (l && !ret) {
        struct ag_component* c = (struct ag_component*)l->data;
        if ((c->name && !strcmp(c->name, name_or_alias)) || 
            (c->alias && !strcmp(c->alias, name_or_alias))) {
            ret = c;
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

static int is_component_up_in_branch(struct ag_project* project, struct ag_component* leaf, const char* name) {
    if (!leaf) {
        return 0;
    }
    if (!strcmp(leaf->name, name) || (leaf->alias && !strcmp(leaf->alias, name))) {
        return 1;
    }
    for (struct list* slist = leaf->build_after; slist; slist = slist->next) {
        if (is_component_up_in_branch(project, ag_find_component(project, (char*)slist->data), name)) {
            return 1;
        }
    }
    return 0;
}

static struct list* fill_build_up_list(struct list* old_root, struct ag_project* project, struct ag_component* component, const char* up_to_component) {

    // TODO: heavy code here and in is_component_up_in_branch() function. Need to rewrite in a more efficient way

    if (!old_root) {
        return NULL;
    }

    struct list* new_root = old_root;
    
    for (struct list* slist = component->build_after; slist; slist = slist->next) {
        const char* s = (char*)slist->data;

        int found = 0;
        for (struct list* l = project->components; l && !found; l = l->next) {
            struct ag_component*c = (struct ag_component*)l->data;
            if (!strcmp(c->name, s)) {
                if (!up_to_component || is_component_up_in_branch(project, c, up_to_component)) {
                    new_root = list_create(c, new_root);
                    new_root = fill_build_up_list(new_root, project, c, up_to_component);
                }
                found = 1;
            }
        }

        if (!found || !new_root) {
            list_free(new_root, NULL);
            return NULL;
        }
    }

    return new_root;
}

static void remove_duplicates(struct list* list) {
    for (struct list* i = list; i; i = i->next) {
        for (struct list* j = i->next, *prev_j = i; j; prev_j = j, j = j->next) {
            struct ag_component* ic = (struct ag_component*)i->data;
            struct ag_component* jc = (struct ag_component*)j->data;
            if (!strcmp(ic->name, jc->name)) {
                prev_j->next = j->next;
                free(j);
                j = prev_j;
            }
        }
    }
}

struct list* ag_build_up_list(struct ag_project* project, struct ag_component* component, const char* up_to_component) {
    assert(project);
    assert(component);
    struct list* ret = fill_build_up_list(list_create(component, NULL), project, component, up_to_component);
    remove_duplicates(ret);
    return ret;
}

static struct list* fill_build_down_list(struct ag_component* root, struct ag_project* project, struct ag_component* down_cmp) {
    if (!root) {
        return NULL;
    }

    struct list* ret = NULL;
    struct list* l = NULL;

    struct list* queue_head = list_create(root, NULL);
    struct list* queue_tail = queue_head;
    while (queue_head) {
        struct ag_component* c = (struct ag_component*)queue_head->data;
        list_add(&ret, &l, c);
        for (struct list* pl = project->components; pl; pl = pl->next) {
            struct ag_component* pc = (struct ag_component*)pl->data;
            for (struct list* sl = pc->build_after; sl; sl = sl->next) {
                if (!strcmp((char*)sl->data, c->name)) {
                    if (!down_cmp || is_component_up_in_branch(project, down_cmp, pc->name)) {
                        list_add(&queue_head, &queue_tail, pc);
                    }
                    break;
                }
            }
        }
        list_pop(&queue_head);
    }
    return ret;
}

struct list* ag_build_down_list(struct ag_project* project, struct ag_component* component, const char* down_to_component) {
    assert(project);
    assert(component);
    struct ag_component* down_cmp = NULL;
    if (down_to_component) {
        down_cmp = ag_find_component(project, down_to_component);
    }
    struct list* ret = fill_build_down_list(component, project, down_cmp);
    remove_duplicates(ret);
    return ret;
}

struct list* ag_build_all_list(struct ag_project* project) {
    assert(project);
    struct list* ret = NULL;
    struct list* tail = NULL;
    for (struct list* i = project->components; i; i = i->next) {
        int found = 0;
        struct ag_component* ic = (struct ag_component*)i->data;
        for (struct list* j = ret, *prev_j = NULL; j; prev_j = j, j = j->next) {
            struct ag_component* jc = (struct ag_component*)j->data;
            if (is_component_up_in_branch(project, jc, ic->name)) {
                struct list* n = list_create(ic, j);
                if (prev_j) {
                    prev_j->next = n;
                } else {
                    ret = n;
                }
                found = 1;
                break;
            }
        }
        if (!found) {
            list_add(&ret, &tail, ic);
        }
    }
    return ret;
}
