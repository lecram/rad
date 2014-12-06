/* gcc -Os -Wall -Wextra -Werror -std=c99 -o y y.c -lm */

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <complex.h>

#define BUFSZ 1024
static char buffer[BUFSZ];

typedef complex number_t;

typedef struct {
    unsigned bulk;
    unsigned len;
    number_t *slots;
} stk_t;

static stk_t stk = {16u, 0u, NULL};

static void
push(number_t number)
{
    if (stk.len >= stk.bulk) {
        stk.bulk += stk.bulk >> 1;
        stk.slots = (number_t *) realloc(stk.slots, stk.bulk * sizeof(number_t));
    }
    stk.slots[stk.len++] = number;
}

static number_t
pop()
{
    return stk.slots[--stk.len];
}

static number_t
peek()
{
    return stk.slots[stk.len - 1];
}

int
snprintn(char *buffer, int buf_size, number_t number)
{
    int count;
    if (cimag(number))
        count = snprintf(buffer, buf_size, "%g;%g", creal(number), cimag(number));
    else
        count = snprintf(buffer, buf_size, "%g", creal(number));
    return count;
}

static number_t
parse(const char *token)
{
    number_t number;
    if (strchr(token, ';')) {
        char *sep;
        number = strtod(token, &sep);
        number += atof(++sep) * I;
    } else if (strchr(token, '.')) {
        number = (number_t) atof(token);
    } else {
        number = (number_t) atoi(token);
    }
    return number;
}

static void
process(const char *token)
{
    number_t a, b;
    if (!strcmp(token, "+")) {
        b = pop();
        a = pop();
        push(a + b);
    } else if (!strcmp(token, "-")) {
        b = pop();
        a = pop();
        push(a - b);
    } else if (!strcmp(token, "*")) {
        b = pop();
        a = pop();
        push(a * b);
    } else if (!strcmp(token, "/")) {
        b = pop();
        a = pop();
        push(a / b);
    } else if (!strcmp(token, "%")) {
        b = pop();
        a = pop();
        push(fmod(creal(a), creal(b)));
    } else if (!strcmp(token, "#")) {
        b = pop();
        a = pop();
        push(cpow(a, b));
    } else if (!strcmp(token, "p")) {
        unsigned i;
        int offset = 0;
        strcpy(buffer, "");
        for (i = 0; i < stk.len; i++, offset++) {
            offset += snprintn(buffer + offset, BUFSZ - offset, stk.slots[i]);
            strcat(buffer, " ");
        }
        puts(buffer);
    } else {
        push(parse(token));
    }
}

int
main()
{
    const char *token;
    stk.slots = (number_t *) malloc(stk.bulk * sizeof(number_t));
    while (!feof(stdin)) {
        if (!fgets(buffer, BUFSZ, stdin))
            break;
        token = strtok(buffer, " \n");
        while (token) {
            process(token);
            token = strtok(NULL, " \n");
        }
        if (stk.len) {
            snprintn(buffer, BUFSZ, peek());
            puts(buffer);
        }
    }
    free(stk.slots);
    return 0;
}
