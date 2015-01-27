
#include "common.h"

#include <assert.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

void (*xexit)(int status) = &exit;

pid_t xfork() {
    pid_t ret = fork();
    if (0 == ret) {
        // child process should call _Exit
        xexit = &_Exit;
    }
    return ret;
}

void die(const char * format, ...) {
    va_list vargs;
    va_start (vargs, format);
    vfprintf (stderr, format, vargs);
    fprintf (stderr, "\n");
    xexit(1);
}

void* xcalloc(size_t count, size_t size) {
    void* ret = calloc(count, size);
    if (!ret) {
        die("Out of memory, calloc failed");
    }
    return ret;
}

void* xmalloc(size_t size) {
    void* ret = malloc(size);
    if (!ret) {
        die("Out of memory, malloc failed");
    }
    return ret;
}

void* xrealloc(void* ptr, size_t size) {
    void* ret = realloc(ptr, size);
    if (!ret) {
        die("Out of memory, realloc failed");
    }
    return ret;
}

char* xstrdup(const char* s1) {
    char* ret = strdup(s1);
    if (!ret) {
        die("Out of memory, strdup failed");
    }
    return ret;
}

char* parent_dir(char* absolute_path) {
    assert('/' == *absolute_path);

    char* t = strrchr(absolute_path, '/');
    *t = '\0';
    char* ret = xstrdup(absolute_path);
    *t = '/';
    if (!ret[0]) {
        free(ret);
        ret = xstrdup("/");
    }
    return ret;
}

int empty(const char* s) {
    return !s || !s[0];
}

int dir_exists(const char* path) {
    DIR* dir = opendir(path);
    if (dir) {
        closedir(dir);
        return 1;
    } else {
        return 0;
    }
}

pid_t run_cmd_line(const char* cmd_line, int supress_output) {
    assert(cmd_line);

    pid_t child_pid = xfork();
    if (0 == child_pid) {
        if (supress_output) {
            fclose(stdout);
            fclose(stderr);
        }
        execl("/bin/sh", "sh", "-c", cmd_line, (char*)NULL);
        return -1;
    }
    return child_pid;
}

char* create_temp_file(const char* prefix, const char* content) {
    char* fname = NULL;
    if (-1 == asprintf(&fname, "/tmp/%sXXXXXXXXXX", prefix)) {
        return NULL;
    }
    if (!mktemp(fname)) {
        free(fname);
        return NULL;
    }
    FILE* f = fopen(fname, "w");
    if (content) {
        fprintf(f, "%s", content);
    }
    fclose(f);
    return fname;
}

pid_t run_script(const char* dir, const char* script_file_name) {
    assert(script_file_name);

    pid_t child_pid = xfork();
    if (0 == child_pid) {
        if (dir && chdir(dir)) {
            return -1;
        }
        execl("/bin/sh", "sh", "-xe", script_file_name, (char*)NULL);
        return -1;
    }
    return child_pid;
}

struct list* list_create(void* data, struct list* next) {
    if (!data) {
        return NULL;
    }
    struct list* ret = (struct list*)xcalloc(1, sizeof(struct list));
    ret->data = data;
    ret->next = next;
    return ret;    
}

void list_free(struct list* list, void (*free_data)(void*)) {
    if (!list) {
        return;
    }
    struct list* n = NULL;
    while (list) {
        if (free_data) {
            free_data(list->data);
        }
        n = list->next;
        free(list);
        list = n;
    }
}

void list_add(struct list** head, struct list** tail, void* data) {
    assert(head);
    assert(tail);

    if (!data) {
        return;
    }
    if (*head) {
        (*tail)->next = list_create(data, NULL);
        (*tail) = (*tail)->next;
    } else {
        *head = list_create(data, NULL);
        *tail = *head;
    }
}

void* list_pop(struct list** head) {
    assert(head);
    if (!*head) {
        return NULL;
    }
    void* ret = (*head)->data;
    struct list* n = *head;
    (*head) = (*head)->next;
    free(n);
    return ret;
}
