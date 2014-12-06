/* gcc -Os -Wall -Wextra -Werror -std=c99 -o y y.c -lm */

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <complex.h>

#define PI 3.14159265358979323846
#define E  2.71828182845904523536

#define BUFSZ 1024
static char buffer[BUFSZ];

typedef enum {DEC, HEX} repr_t;
static repr_t repr = DEC;

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

static void
exch()
{
    number_t n;
    n = stk.slots[stk.len - 1];
    stk.slots[stk.len - 1] = stk.slots[stk.len - 2];
    stk.slots[stk.len - 2] = n;
}

int
snprintn(char *buf, int buf_size, number_t number)
{
    int count = 0;
    switch (repr) {
        case DEC:
            if (cimag(number))
                count = snprintf(buf, buf_size, "%g;%g", creal(number), cimag(number));
            else
                count = snprintf(buf, buf_size, "%g", creal(number));
            break;
        case HEX:
            count = snprintf(buf, buf_size, "%#0*X\n", sizeof(unsigned) * 2 + 2, (unsigned) number);
            break;
    }
    return count;
}

static number_t
parse(const char *token)
{
    number_t number;
    if (!strcmp(token, "e"))
        return E;
    else if (!strcmp(token, "pi"))
        return PI;
    else if (!strcmp(token, "i"))
        return I;
    else if (strchr(token, ';')) {
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
    } else if (!strcmp(token, "~")) {
        push(~ (unsigned) pop());
    } else if (!strcmp(token, "|")) {
        b = pop();
        a = pop();
        push(((unsigned) a) | ((unsigned) b));
    } else if (!strcmp(token, "&")) {
        b = pop();
        a = pop();
        push(((unsigned) a) & ((unsigned) b));
    } else if (!strcmp(token, "^")) {
        b = pop();
        a = pop();
        push(((unsigned) a) ^ ((unsigned) b));
    } else if (!strcmp(token, "real")) {
        push(creal(pop()));
    } else if (!strcmp(token, "imag")) {
        push(cimag(pop()));
    } else if (!strcmp(token, "arg")) {
        push(carg(pop()));
    } else if (!strcmp(token, "abs")) {
        push(cabs(pop()));
    } else if (!strcmp(token, "conj")) {
        push(conj(pop()));
    } else if (!strcmp(token, "proj")) {
        push(cproj(pop()));
    } else if (!strcmp(token, "exp")) {
        push(cexp(pop()));
    } else if (!strcmp(token, "log")) {
        push(clog(pop()));
    } else if (!strcmp(token, "sqrt")) {
        push(csqrt(pop()));
    } else if (!strcmp(token, "acos")) {
        push(cacos(pop()));
    } else if (!strcmp(token, "asin")) {
        push(casin(pop()));
    } else if (!strcmp(token, "atan")) {
        push(catan(pop()));
    } else if (!strcmp(token, "cos")) {
        push(ccos(pop()));
    } else if (!strcmp(token, "sin")) {
        push(csin(pop()));
    } else if (!strcmp(token, "tan")) {
        push(ctan(pop()));
    } else if (!strcmp(token, "acosh")) {
        push(cacosh(pop()));
    } else if (!strcmp(token, "asinh")) {
        push(casinh(pop()));
    } else if (!strcmp(token, "atanh")) {
        push(catanh(pop()));
    } else if (!strcmp(token, "cosh")) {
        push(ccosh(pop()));
    } else if (!strcmp(token, "sinh")) {
        push(csinh(pop()));
    } else if (!strcmp(token, "tanh")) {
        push(ctanh(pop()));
    } else if (!strcmp(token, "drop")) {
        pop();
    } else if (!strcmp(token, "dup")) {
        push(peek());
    } else if (!strcmp(token, "exch")) {
        exch();
    } else if (!strcmp(token, "clear")) {
        while (stk.len) pop();
    } else
        push(parse(token));
}

int
main()
{
    const char *token;
    stk.slots = (number_t *) malloc(stk.bulk * sizeof(number_t));
    while (!feof(stdin)) {
        if (!fgets(buffer, BUFSZ, stdin))
            break;
        if (!strcmp(buffer, "show\n")) {
            unsigned i;
            int offset;
            for (i = 0, offset = 0; i < stk.len; i++, offset++) {
                offset += snprintn(buffer + offset, BUFSZ - offset, stk.slots[i]);
                strcat(buffer, " ");
            }
            puts(buffer);
        } else if (!strcmp(buffer, "dec\n")) {
            repr = DEC;
        } else if (!strcmp(buffer, "hex\n")) {
            repr = HEX;
        } else {
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
    }
    free(stk.slots);
    return 0;
}
