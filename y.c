/* gcc -Os -Wall -Wextra -Werror -std=c99 -o y y.c -lm */

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <complex.h>

typedef complex number_t;

typedef struct {
    unsigned bulk;
    unsigned len;
    number_t *slots;
} stk_t;

static stk_t stk = {16u, 0, NULL};

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
snprinto(char *buffer, int buf_size, number_t number)
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
    } else {
        push(parse(token));
    }
}

#define BUFSZ 1024

int
main()
{
    char buffer[BUFSZ];
    const char *token;
    int offset;
    stk.slots = (number_t *) malloc(stk.bulk * sizeof(number_t));
    while (!feof(stdin)) {
        if (!fgets(buffer, BUFSZ, stdin))
            break;
        token = strtok(buffer, " \n");
        while (token) {
            process(token);
            token = strtok(NULL, " \n");
        }
        snprinto(buffer, BUFSZ, peek());
        puts(buffer);
    }
    offset = 0;
    while (stk.len) {
        offset += snprinto(buffer + offset, BUFSZ - offset, pop()) + 1;
        strcat(buffer, " ");
    }
    puts(buffer);
    free(stk.slots);
    return 0;
}
