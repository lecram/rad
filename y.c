/* gcc -Wall -Wextra -pedantic -std=c99 -o y y.c */

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <complex.h>

typedef enum {INTEGER, REAL, COMPLEX} tag_t;
typedef tag_t obj_t;

typedef struct {
    tag_t tag;
    int val;
} i_t;

typedef struct {
    tag_t tag;
    double val;
} r_t;

typedef struct {
    tag_t tag;
    complex val;
} c_t;

typedef struct {
    unsigned bulk;
    unsigned len;
    obj_t **objs;
} stk_t;

static stk_t stk = {16u, 0, NULL};

static void
push(obj_t *obj)
{
    if (stk.len >= stk.bulk) {
        stk.bulk += stk.bulk >> 1;
        stk.objs = (obj_t **) realloc(stk.objs, stk.bulk * sizeof(obj_t *));
    }
    stk.objs[stk.len++] = obj;
}

static void
pop()
{
    free(stk.objs[--stk.len]);
}

static obj_t *
peek()
{
    return stk.objs[stk.len - 1];
}

static void
clear()
{
    while (stk.len) pop();
}

int
snprinto(char *buffer, int buf_size, obj_t *obj)
{
    switch ((tag_t) *obj) {
        case INTEGER:
            return snprintf(
                buffer, buf_size, "%d",
                ((i_t *) obj)->val
            );
        case REAL:
            return snprintf(
                buffer, buf_size, "%f",
                ((r_t *) obj)->val
            );
        case COMPLEX:
            return snprintf(
                buffer, buf_size, "%f;%f",
                creal(((c_t *) obj)->val), cimag(((c_t *) obj)->val)
            );
    }
    return 0;
}

static obj_t *
parse(const char *token)
{
    obj_t *obj;
    if (strchr(token, ';')) {
        char *sep;
        c_t *c = (c_t *) malloc(sizeof(c_t));
        c->tag = COMPLEX;
        c->val = strtod(token, &sep);
        c->val += atof(++sep) * I;
        obj = (obj_t *) c;
    } else if (strchr(token, '.')) {
        r_t *r = (r_t *) malloc(sizeof(r_t));
        r->tag = REAL;
        r->val = atof(token);
        obj = (obj_t *) r;
    } else {
        i_t *i = (i_t *) malloc(sizeof(i_t));
        i->tag = INTEGER;
        i->val = atoi(token);
        obj = (obj_t *) i;
    }
    return obj;
}

#define BUFSZ 1024

int
main()
{
    char buffer[BUFSZ];
    stk.objs = (obj_t **) malloc(stk.bulk * sizeof(obj_t *));
    push(parse("-13"));
    push(parse("3.14"));
    push(parse("2.56;-5.23"));
    while (stk.len) {
        snprinto(buffer, BUFSZ, peek());
        puts(buffer);
        pop();
    }
    clear();
    free(stk.objs);
    return 0;
}
