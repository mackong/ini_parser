#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <stdlib.h>

#include "list.h"
#include "ini_parser.h"

struct ini_element {
        char *name; /** element name */
        char *value; /** element value */
        LIST_ENTRY(ini_element) field;
};

struct ini_section {
        char *name; /** section name */
        LIST_ENTRY(ini_section) field;
        LIST_HEAD(elem_head, ini_element) elem_head;
};

struct ini_context {
        char *name; /** context name, just filename here */
        LIST_HEAD(sec_head, ini_section) sec_head;
};
        
static struct ini_context *parse_file(FILE *fp)
{
        char *line = NULL;
        size_t len = 0;
        ssize_t read = 0;
        struct ini_context *ctx = NULL;
        struct ini_section *last_sec = NULL;

        ctx = (struct ini_context *)malloc(sizeof(*ctx));
        if (!ctx) {
                return NULL;
        }
        LIST_INIT(&ctx->sec_head);
        
        while ((read = getline(&line, &len, fp)) != -1) {
                char *p = line;

                while (*p == ' ')p++; /** trim leading whitespaces */
                if (strlen(p) <= 1) continue; /** skip blank line */
                else if (*p == '#') continue; /** skip comment */
                else if (*p == '[') { /** section start */
                        p++; /** skip [ */
                        last_sec = (struct ini_section *)
                                malloc(sizeof(*last_sec));
                        LIST_INIT(&last_sec->elem_head);
                        if (!last_sec) continue;
                        /** trim whitespaces between [ and section name */
                        while (*p == ' ')p++;
                        /** copy section name */
                        last_sec->name = strdup(p);
                        /** trim whitespaces between section name and ] */
                        p = last_sec->name;
                        while (*p) {
                                if (*p == ' ' || *p == ']') {
                                        *p = '\0';
                                        break;
                                }
                                p++;
                        }

                        /** Insert the section to context */
                        LIST_INSERT_HEAD(&ctx->sec_head, last_sec, field);
                } else { /** element */
                        struct ini_element *elem = NULL;
                        char *tmp = NULL;

                        elem = (struct ini_element *)malloc(sizeof(*elem));
                        tmp = strtok(p, "=");
                        elem->name = strdup(tmp);
                        tmp = strtok(NULL, "=");
                        elem->value = strdup(tmp);
                        /** trim whitespaces for name and value */
                        tmp = elem->name;
                        while (*tmp) {
                                if (*tmp == ' ') {
                                        *tmp = '\0';
                                        break;
                                }
                                tmp++;
                        }
                        tmp = elem->value;
                        while (*tmp) {
                                if (*tmp == '\n') {
                                        *tmp = '\0';
                                        break;
                                }
                                tmp++;
                        }
                        LIST_INSERT_HEAD(&last_sec->elem_head, elem, field);
                }
        }
        free(line);
        return ctx;
}

struct ini_context *ini_load(const char *filename)
{
        struct ini_context *ctx = NULL;
        FILE *fp = NULL;

        if (!filename) {
                return NULL;
        }
        
        fp = fopen(filename, "r");
        if (!fp) {
                return NULL;
        }

        ctx = parse_file(fp);
        if (ctx) {
                ctx->name = strdup(filename);
        }

        fclose(fp);
        return ctx;
}

void ini_unload(struct ini_context *ctx)
{
        struct ini_section *sec_head = NULL;
        struct ini_element *elem_head = NULL;
        
        if (!ctx) {
                return;
        }

        while (sec_head = LIST_FIRST(&(ctx->sec_head))) {
                struct ini_section *sec = LIST_NEXT(sec_head, field);
                LIST_REMOVE(sec_head, field);
                while (elem_head = LIST_FIRST(&(sec_head->elem_head))) {
                        struct ini_element *elem = LIST_NEXT(elem_head, field);
                        LIST_REMOVE(elem_head, field);
                        free(elem_head->name);
                        free(elem_head->value);
                        free(elem_head);
                        elem_head = elem;
                }
                free(sec_head->name);
                free(sec_head);
                sec_head = sec;
        }

        free(ctx->name);
        free(ctx);
        ctx = NULL;
}

char *ini_get_string(struct ini_context *ctx, const char *sec_name,
                     const char *elem_name, char *default_value)
{
        struct ini_section *sec = NULL;
        struct ini_element *elem = NULL;

        if (!ctx || !sec_name || !elem_name) {
                return default_value;
        }

        LIST_FOREACH(sec, &(ctx->sec_head), field) {
                if (strcmp(sec->name, sec_name) != 0) {
                        continue;
                }
                LIST_FOREACH(elem, &(sec->elem_head), field) {
                        if (strcmp(elem->name, elem_name) == 0) {
                                return elem->value;
                        }
                }
        }
        return default_value;
}

int ini_get_int(struct ini_context *ctx, const char *sec_name,
                const char *elem_name, int default_value)
{
        char *value = ini_get_string(ctx, sec_name, elem_name, NULL);
        if (!value) {
                return default_value;
        }
        return atoi(value);
}

int ini_get_bool(struct ini_context *ctx, const char *sec_name,
                 const char *elem_name, int default_value)
{
        char *value = ini_get_string(ctx, sec_name, elem_name, NULL);
        if (!value) {
                return default_value;
        }
        if (strcasecmp(value, "true") == 0) {
                return 1;
        } else if (strcasecmp(value, "false") == 0) {
                return 0;
        } else {
                return atoi(value) > 0;
        }
}

#ifdef DEBUG

int main(int argc, char **argv)
{
        struct ini_context *ctx = NULL;
        
        ctx = ini_load(argv[1]);
        printf("name = %s\n", ini_get_string(ctx, "test", "name", "mackong"));
        ini_unload(ctx);
        
        return 0;
}

#endif /** DEBUG */
