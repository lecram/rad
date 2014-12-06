/* gcc -Wall -Wextra -pedantic -std=c99 -o y y.c */

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <complex.h>

#define MAX(A, B) ((A) > (B) ? (A) : (B))

typedef int integer_t;
typedef double real_t;
typedef complex complex_t;
typedef enum {INTEGER, REAL, COMPLEX} tag_t;
typedef tag_t obj_t;

typedef struct {
    tag_t tag;
    integer_t val;
} i_t;

typedef struct {
    tag_t tag;
    real_t val;
} r_t;

typedef struct {
    tag_t tag;
    complex_t val;
} c_t;

#define VAL(O) ((complex_t) (*(O) == INTEGER ? ((i_t *) (O))->val : \
               (*(O) == REAL ? ((r_t *) (O))->val : \
               (*(O) == COMPLEX ? ((c_t *) (O))->val : 0))))

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

static obj_t *
pop()
{
    return stk.objs[--stk.len];
}

static obj_t *
peek()
{
    return stk.objs[stk.len - 1];
}

static void
clear()
{
    while (stk.len) free(pop());
}

obj_t *
new(tag_t tag, complex_t val)
{
    obj_t *obj;
    switch (tag) {
        i_t *i;
        r_t *r;
        c_t *c;
        case INTEGER:
            i = (i_t *) malloc(sizeof(i_t));
            i->val = (integer_t) val;
            obj = (obj_t *) i;
            break;
        case REAL:
            r = (r_t *) malloc(sizeof(r_t));
            r->val = (real_t) val;
            obj = (obj_t *) r;
            break;
        case COMPLEX:
            c = (c_t *) malloc(sizeof(c_t));
            c->val = val;
            obj = (obj_t *) c;
            break;
    }
    *obj = tag;
    return obj;
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
        complex_t val = strtod(token, &sep);
        val += atof(++sep) * I;
        obj = new(COMPLEX, val);
    } else if (strchr(token, '.')) {
        real_t val = atof(token);
        obj = new(REAL, (complex_t) val);
    } else {
        integer_t val = atoi(token);
        obj = new(INTEGER, (complex_t) val);
    }
    return obj;
}

static void
process(const char *token)
{
    obj_t *a, *b;
    tag_t tag;
    if (!strcmp(token, "+")) {
        b = pop();
        a = pop();
        tag = MAX((tag_t) *a, (tag_t) *b);
        push(new(tag, VAL(a) + VAL(b)));
    } else if (!strcmp(token, "-")) {
        b = pop();
        a = pop();
        tag = MAX((tag_t) *a, (tag_t) *b);
        push(new(tag, VAL(a) - VAL(b)));
    } else if (!strcmp(token, "*")) {
        b = pop();
        a = pop();
        tag = MAX((tag_t) *a, (tag_t) *b);
        push(new(tag, VAL(a) * VAL(b)));
    } else if (!strcmp(token, "/")) {
        b = pop();
        a = pop();
        tag = MAX((tag_t) *a, (tag_t) *b);
        push(new(tag, VAL(a) / VAL(b)));
    } else if (!strcmp(token, "%")) {
        b = pop();
        a = pop();
        tag = MAX((tag_t) *a, (tag_t) *b);
        push(new(tag, (complex_t) (((integer_t) VAL(a)) % ((integer_t) VAL(b)))));
    } else {
        push(parse(token));
    }
}

#define BUFSZ 1024

int
main()
{
    char buffer[BUFSZ];
    stk.objs = (obj_t **) malloc(stk.bulk * sizeof(obj_t *));
    process("-13");
    process("5.3;1");
    process("+");
    /* process("3.14"); */
    /* process("2.56;-5.23"); */
    while (stk.len) {
        snprinto(buffer, BUFSZ, peek());
        puts(buffer);
        free(pop());
    }
    clear();
    free(stk.objs);
    return 0;
}
