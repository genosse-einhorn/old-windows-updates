#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

static inline char **
parse_args(const char *argstr, int *pargc)
{
    int argc = 1;
    int numchars = 0;
    const char *str = argstr;

    /* step 1: count arguments and characters */

    // program name is special
    if (*str == '"') {
        ++str;
        while (*str != '"' && *str) {
            ++str;
            ++numchars;
        }

        if (*str == '"')
            ++str; // skip over "
    } else {
        while (*str != ' ' && *str != '\t' && *str) {
            ++str;
            ++numchars;
        }
    }

    while (*str == ' ' || *str == '\t')
        ++str;

    // now the arguments
    while (*str) {
        ++argc;

        int iq = 0;
        while (*str && (iq || (*str != ' ' && *str != '\t'))) {
            if (*str == '"' && !iq) {
                iq = 1;
                ++str;
            } else if (*str == '"' && iq) {
                ++str;
                if (*str == '"') {
                    ++numchars;
                    ++str; // remain in quotes mode
                } else {
                    iq = 0;
                }
            } else if (*str == '\\') {
                int bc = 1;
                ++str;
                while (*str == '\\') {
                    ++bc;
                    ++str;
                }
                if (*str == '"') {
                    if (bc % 2 == 1) { // escaped double quote
                        ++str;
                        numchars += bc / 2 + 1;
                    } else {
                        numchars += bc / 2;
                    }
                } else {
                    numchars += bc;
                }
            } else {
                ++str;
                ++numchars;
            }
        }

        while (*str == ' ' || *str == '\t')
            ++str;
    }

    /* step 2: create argument array */
    char *buf = malloc((argc + 1) * sizeof(char*) + argc + numchars);
    char **argv = (char **)buf;
    buf += (argc + 1) * sizeof(char*);

    char **pargi = argv;
    str = argstr;

    // program name is special
    *pargi = buf;
    if (*str == '"') {
        ++str;
        while (*str != '"' && *str) {
            *buf++ = *str++;
        }

        if (*str == '"')
            ++str; // skip over "
    } else {
        while (*str != ' ' && *str != '\t' && *str)
            *buf++ = *str++;
    }
    *buf++ = 0;

    while (*str == ' ' || *str == '\t')
        ++str;

    // now the arguments
    while (*str) {
        *++pargi = buf;

        int iq = 0;
        while (*str && (iq || (*str != ' ' && *str != '\t'))) {
            if (*str == '"' && !iq) {
                iq = 1;
                ++str;
            } else if (*str == '"' && iq) {
                ++str;
                if (*str == '"') {
                    *buf++ = '"';
                    ++str; // remain in quotes mode
                } else {
                    iq = 0;
                }
            } else if (*str == '\\') {
                int bc = 1;
                ++str;
                while (*str == '\\') {
                    ++bc;
                    ++str;
                }
                if (*str == '"') {
                    if (bc % 2 == 1) { // escaped double quote
                        ++str;
                        for (int i = 0; i < bc/2; ++i)
                            *buf++ = '\\';
                        *buf++ = '"';
                    } else {
                        for (int i = 0; i < bc/2; ++i)
                            *buf++ = '\\';
                    }
                } else {
                    for (int i = 0; i < bc; ++i)
                        *buf++ = '\\';
                }
            } else {
                *buf++ = *str++;
            }
        }

        *buf++ = 0;

        while (*str == ' ' || *str == '\t')
            ++str;
    }

    *++pargi = NULL;

    if (pargc)
        *pargc = argc;

    return argv;
}

static inline int count_args(const char *str) {
    int argc;
    char **argv = parse_args(str, &argc);
    free(argv);
    return argc;
}

static inline int
strv_len(char **strv)
{
    int i = 0;
    while (*strv++)
        ++i;
    return i;
}

static inline void
print_unequal_strv(char **strv1, char **strv2)
{
    printf("[");
    for (char **ps = strv1; *ps; ++ps) {
        printf("%s", *ps);
        if (ps[1])
            printf(", ");
    }
    printf("] != [");
    for (char **ps = strv2; *ps; ++ps) {
        printf("%s", *ps);
        if (ps[1])
            printf(", ");
    }
    printf("]\n");
}

static inline void
check_strv_equal(char **strv1, char **strv2)
{
    if (strv_len(strv1) != strv_len(strv2)) {
        print_unequal_strv(strv1, strv2);
    } else {
        for (int i = 0; strv1[i] && strv2[i]; ++i) {
            if (strcmp(strv1[i], strv2[i])) {
                print_unequal_strv(strv1, strv2);
                break;
            }
        }
    }
}

static inline void
check_parse_args(const char *argstr, char **argv)
{
    int c;
    char **a = parse_args(argstr, &c);
    assert(c == strv_len(a));
    check_strv_equal(a, argv);
    free(a);
}

static inline size_t
quote_arg(const char *arg, char *buf, size_t bufsize)
{
    size_t r = 1;
#define WRITE(c) do { r++; if (bufsize > 1) { *buf++ = (c); bufsize--; } } while (0)

    WRITE('"');

    while (*arg) {
        char c = *arg++;

        if (c == '"') {
            WRITE('\\');
            WRITE('"');
        } else if (c == '\\') {
            size_t b = 1;
            while (*arg == '\\') {
                arg++;
                b++;
            }
            if (*arg == '"') {
                ++arg;
                for (size_t i = 0; i < b; ++i) {
                    WRITE('\\'); WRITE('\\');
                }
                WRITE('\\');
                WRITE('"');
            } else if (*arg == '\0') {
                for (size_t i = 0; i < b; ++i) {
                    WRITE('\\'); WRITE('\\');
                }
            } else {
                for (size_t i = 0; i < b; ++i) {
                    WRITE('\\');
                }
            }
        } else {
            WRITE(c);
        }
    }

    WRITE('"');

#undef WRITE

    if (bufsize > 0)
        *buf = 0;

    return r;
}

static inline void
check_quote_arg(const char *arg, const char *quoted)
{
    char buf[1024];

    quote_arg(arg, buf, sizeof(buf));

    assert(!strcmp(buf, quoted));
}

int main(void)
{
    check_parse_args("\"abc\" d e", ((char*[]){ "abc", "d", "e", NULL }));
    check_parse_args("x a\\\\\\b d\"e f\"g h", (char*[]){"x", "a\\\\\\b", "de fg", "h", NULL});
    check_parse_args("test.exe a\\\\\\\"b c d", (char*[]){"test.exe", "a\\\"b", "c", "d", NULL});
    check_parse_args("x a\\\\\\\\\"b c\" d e", (char*[]){"x", "a\\\\b c", "d", "e", NULL});
    check_parse_args("\"a b\\\"cde\"", (char*[]){"a b\\", "cde", NULL});
    check_parse_args("test.exe \"c:\\Path With Spaces\\Ending In Backslash\\\" Arg2 Arg3", (char*[]){"test.exe", "c:\\Path With Spaces\\Ending In Backslash\" Arg2 Arg3", NULL});
    check_parse_args("test.exe \"c:\\Path With Spaces\\Ending In Backslash\\\\\" Arg2 Arg3", (char*[]){"test.exe", "c:\\Path With Spaces\\Ending In Backslash\\", "Arg2", "Arg3", NULL});
    check_parse_args("DumpArgs foo\"\"\"\"\"\"\"\"\"\"\"\"bar", (char*[]){"DumpArgs", "foo\"\"\"\"\"bar", NULL});
    check_parse_args("", (char*[]){"", NULL});
    check_parse_args(" \t ", (char*[]){"", NULL});
    check_parse_args("test.exe", (char*[]){"test.exe", NULL});
    check_parse_args("test.exe     ", (char*[]){"test.exe", NULL});

    check_quote_arg("abc", "\"abc\"");
    check_quote_arg("", "\"\"");
    check_quote_arg("abc\\", "\"abc\\\\\"");
    check_quote_arg("a\\\"b", "\"a\\\\\\\"b\"");

    return 0;
}
