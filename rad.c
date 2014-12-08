#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <complex.h>

#define PI 3.14159265358979323846
#define E  2.71828182845904523536

#define BUFSZ 1024
static char buffer[BUFSZ];

typedef enum {DEC, HEX} repr_t;
static repr_t repr = DEC;
static int prec = 8;

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
pop() { return stk.len ? stk.slots[--stk.len] : 0; }

static number_t
peek() { return stk.len ? stk.slots[stk.len - 1] : 0; }

static void
exch()
{
    number_t n;
    n = stk.slots[stk.len - 1];
    stk.slots[stk.len - 1] = stk.slots[stk.len - 2];
    stk.slots[stk.len - 2] = n;
}

static long
gcd(long a, long b)
{
    long t;
    while (a && b) {
        t = a;
        a = b;
        b = t % b;
    }
    return a;
}

static long
lcm(long a, long b) { return a * b / gcd(a, b); }

static long
egcd(long a, long b)
{
    long q, r, x, y;
    long x1, y1, x2, y2;
    if (!(a && b)) {
        push(1);
        push(0);
    } else {
        x1 = 0; y1 = 1;
        x2 = 1; y2 = 0;
        while (b) {
            q = a / b;
            r = a % b;
            x = x2 - q * x1;
            y = y2 - q * y1;
            a = b; b = r;
            x2 = x1; x1 = x;
            y2 = y1; y1 = y;
        }
        push(x2);
        push(y2);
    }
    return a;
}

static unsigned
factorial(unsigned n)
{
    long f = 1;
    while (n) f *= n--;
    return f;
}

static unsigned
tot(unsigned n)
{
    unsigned tot = 1;
    unsigned pos = n - 1;
    while (pos > 1) {
        if (gcd((long) pos, (long) n) == 1)
            tot++;
        pos--;
    }
    return tot;
}

static unsigned
isprime(unsigned n)
{
    unsigned d;
    unsigned m;
    if (n < 2)
        return 0;
    if (n == 2)
        return 1;
    if (n % 2 == 0)
        return 0;
    m = (unsigned) sqrt((double) n);
    for (d = 3; d <= m; d += 2)
        if (n % d == 0)
            return 0;
    return 1;
}

static unsigned
prev(unsigned n)
{
    if (n < 4)
        return 2;
    if (n % 2 == 0)
        n++;
    for (n -= 2; n > 2; n -= 2)
        if (isprime(n))
            break;
    return n;
}

static unsigned
next(unsigned n)
{
    if (n < 2)
        return 2;
    if (n == 2)
        return 3;
    if (n % 2 == 0)
        n--;
    for (n += 2; n > 0; n += 2)
        if (isprime(n))
            break;
    return n;
}

static unsigned
pn(unsigned n)
{
    unsigned p = 1;
    while (n--) p = next(p);
    return p;
}

static unsigned
primorial(unsigned n)
{
    unsigned r;
    r = isprime(n) ? n : 1;
    while (n > 2) {
        n = prev(n);
        r *= n;
    }
    return r;
}

static unsigned
factor(unsigned n)
{
    unsigned d;
    unsigned c = 0;
    if (n % 2 == 0) {
        push(2);
        c++;
        while (n % 2 == 0) n /= 2;
    }
    for (d = 3; d <= n; d += 2) {
        if (n % d == 0) {
            push(d);
            c++;
            while (n % d == 0) n /= d;
        }
    }
    return c;
}

static int
snprintn(char *buf, int buf_size, number_t number)
{
    int count = 0;
    switch (repr) {
        case DEC:
            if (cimag(number))
                count = snprintf(
                    buf, buf_size, "%.*g;%.*g",
                    prec, creal(number), prec, cimag(number)
                );
            else
                count = snprintf(
                    buf, buf_size, "%.*g",
                    prec, creal(number)
                );
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
    } else if (strchr(token, '.'))
        number = (number_t) atof(token);
    else
        number = (number_t) strtol(token, NULL, 0);
    return number;
}

#define BINOP(OP)   else if (!strcmp(token, #OP)) \
                    { b = pop(); a = pop(); push(a OP b); }

#define BINBOP(OP)  else if (!strcmp(token, #OP)) \
                    { b = pop(); a = pop(); push(((unsigned) a) OP ((unsigned) b)); }

#define BFUNC(OP)   else if (!strcmp(token, #OP)) \
                    { b = pop(); a = pop(); push(OP((long) a, (long) b)); }

#define CFUNC(OP)   else if (!strcmp(token, #OP)) \
                    { push(c ## OP(pop())); }

#define UFUNC(OP)   else if (!strcmp(token, #OP)) \
                    { push(OP((unsigned) pop())); }

static void
process(const char *token)
{
    number_t a, b;
    if (!strcmp(token, "prec"))
        prec = pop();
    BINOP(+) BINOP(-) BINOP(*) BINOP(/)
    BINBOP(|) BINBOP(&) BINBOP(^) BINBOP(>>) BINBOP(<<)
    BFUNC(gcd) BFUNC(lcm) BFUNC(egcd)
    CFUNC(real) CFUNC(imag) CFUNC(arg) CFUNC(abs) CFUNC(proj) CFUNC(exp)
    CFUNC(log) CFUNC(sqrt) CFUNC(acos) CFUNC(asin) CFUNC(atan) CFUNC(cos)
    CFUNC(sin) CFUNC(tan) CFUNC(acosh) CFUNC(asinh) CFUNC(atanh) CFUNC(cosh)
    CFUNC(sinh) CFUNC(tanh)
    UFUNC(tot) UFUNC(factor) UFUNC(isprime) UFUNC(prev) UFUNC(next) UFUNC(pn)
    else if (!strcmp(token, "%")) {
        b = pop(); a = pop();
        push(fmod(creal(a), creal(b)));
    } else if (!strcmp(token, "**")) {
        b = pop(); a = pop();
        push(cpow(a, b));
    } else if (!strcmp(token, "~")) {
        push(~ (unsigned) pop());
    } else if (!strcmp(token, "floor")) {
        push(floor(pop()));
    } else if (!strcmp(token, "ceil")) {
        push(ceil(pop()));
    } else if (!strcmp(token, "deg")) {
        push(creal(pop()) * 180 / PI);
    } else if (!strcmp(token, "rad")) {
        push(creal(pop()) * PI / 180);
    } else if (!strcmp(token, "atan2")) {
        b = pop(); a = pop();
        push(atan2(creal(a), creal(b)));
    } else if (!strcmp(token, "conj")) {
        push(conj(pop()));
    } else if (!strcmp(token, "!")) {
        push(factorial((unsigned) pop()));
    } else if (!strcmp(token, "#")) {
        push(primorial((unsigned) pop()));
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
