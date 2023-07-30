/*
MIT License

Copyright (c) 2023 Vladimir Pleskonjic

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

// @TODO doc comments

#ifndef INCLUDE_UNARGS_H
#define INCLUDE_UNARGS_H

#include <stdbool.h>

typedef struct unargs_Param unargs_Param;

unargs_Param unargs_bool(
    const char *name, const char *desc, bool *dst);

unargs_Param unargs_int(
    const char *name, const char *desc, int def, int *dst);

unargs_Param unargs_intReq(
    const char *name, const char *desc, int *dst);

unargs_Param unargs_unsigned(
    const char *name, const char *desc, unsigned def, unsigned *dst);

unargs_Param unargs_unsignedReq(
    const char *name, const char *desc, unsigned *dst);

unargs_Param unargs_long(
    const char *name, const char *desc, long def, long *dst);

unargs_Param unargs_longReq(
    const char *name, const char *desc, long *dst);

unargs_Param unargs_float(
    const char *name, const char *desc, float def, float *dst);

unargs_Param unargs_floatReq(
    const char *name, const char *desc, float *dst);

unargs_Param unargs_double(
    const char *name, const char *desc, double def, double *dst);

unargs_Param unargs_doubleReq(
    const char *name, const char *desc, double *dst);

unargs_Param unargs_string(
    const char *name, const char *desc, const char *def, const char **dst);

unargs_Param unargs_stringReq(
    const char *name, const char *desc, const char **dst);

int unargs_parse(
    int argc, char * const *argv,
    int len, unargs_Param *params);

void unargs_help(const char *program, int len, const unargs_Param *params);

// Below are implementation details.

enum unargs__Type {
    unargs__typeBool,
    unargs__typeInt,
    unargs__typeUnsigned,
    unargs__typeLong,
    unargs__typeFloat,
    unargs__typeDouble,
    unargs__typeString
};

struct unargs_Param {
    const char *_name;
    enum unargs__Type _type;
    bool _req;
    const char *_desc;
    union {
        bool b;
        int i;
        unsigned u;
        long l;
        float f;
        double d;
        const char *str;
    } _def;

    void *_dst;

    bool _found;
};

#endif // INCLUDE_UNARGS_H

// IMPLEMENTATION

#ifdef UNARGS_IMPLEMENTATION

#include <stdlib.h>
#include <string.h>

#if !defined(UNARGS_ASSERT)
#include <assert.h>
#define UNARGS_ASSERT(x) assert(x)
#endif

#if !defined(UNARGS_PRINT_OUT_INT)
#include <stdio.h>
#define UNARGS_PRINT_OUT_INT(x) fprintf(stdout, "%d", x)
#endif

#if !defined(UNARGS_PRINT_OUT_UNSIGNED)
#include <stdio.h>
#define UNARGS_PRINT_OUT_UNSIGNED(x) fprintf(stdout, "%u", x)
#endif

#if !defined(UNARGS_PRINT_OUT_LONG)
#include <stdio.h>
#define UNARGS_PRINT_OUT_LONG(x) fprintf(stdout, "%ld", x)
#endif

#if !defined(UNARGS_PRINT_OUT_FLOAT)
#include <stdio.h>
#define UNARGS_PRINT_OUT_FLOAT(x) fprintf(stdout, "%f", x)
#endif

#if !defined(UNARGS_PRINT_OUT_DOUBLE)
#include <stdio.h>
#define UNARGS_PRINT_OUT_DOUBLE(x) fprintf(stdout, "%f", x)
#endif

#if !defined(UNARGS_PRINT_OUT_STR)
#include <stdio.h>
#define UNARGS_PRINT_OUT_STR(x) fprintf(stdout, "%s", x)
#endif

#if !defined(UNARGS_PRINT_OUT_TAB)
#include <stdio.h>
#define UNARGS_PRINT_OUT_TAB() fprintf(stdout, "\t")
#endif

#if !defined(UNARGS_PRINT_OUT_LN)
#include <stdio.h>
#define UNARGS_PRINT_OUT_LN() fprintf(stdout, "\n")
#endif

#if !defined(UNARGS_PRINT_ERR_INT)
#include <stdio.h>
#define UNARGS_PRINT_ERR_INT(x) fprintf(stderr, "%d", x)
#endif

#if !defined(UNARGS_PRINT_ERR_UNSIGNED)
#include <stdio.h>
#define UNARGS_PRINT_ERR_UNSIGNED(x) fprintf(stderr, "%u", x)
#endif

#if !defined(UNARGS_PRINT_ERR_LONG)
#include <stdio.h>
#define UNARGS_PRINT_ERR_LONG(x) fprintf(stderr, "%ld", x)
#endif

#if !defined(UNARGS_PRINT_ERR_FLOAT)
#include <stdio.h>
#define UNARGS_PRINT_ERR_FLOAT(x) fprintf(stderr, "%f", x)
#endif

#if !defined(UNARGS_PRINT_ERR_DOUBLE)
#include <stdio.h>
#define UNARGS_PRINT_ERR_DOUBLE(x) fprintf(stderr, "%f", x)
#endif

#if !defined(UNARGS_PRINT_ERR_STR)
#include <stdio.h>
#define UNARGS_PRINT_ERR_STR(x) fprintf(stderr, "%s", x)
#endif

#if !defined(UNARGS_PRINT_ERR_TAB)
#include <stdio.h>
#define UNARGS_PRINT_ERR_TAB() fprintf(stderr, "\t")
#endif

#if !defined(UNARGS_PRINT_ERR_LN)
#include <stdio.h>
#define UNARGS_PRINT_ERR_LN() fprintf(stderr, "\n")
#endif

unargs_Param unargs_bool(
    const char *name, const char *desc, bool *dst) {
    UNARGS_ASSERT(name != NULL);
    UNARGS_ASSERT(strlen(name) > 0);

    unargs_Param param;
    param._name = name;
    param._type = unargs__typeBool;
    param._req = false;
    param._desc = desc;
    param._def.b = false;
    param._dst = dst;

    return param;
}

unargs_Param unargs_int(
    const char *name, const char *desc, int def, int *dst) {
    if (name != NULL) UNARGS_ASSERT(strlen(name) > 0);

    unargs_Param param;
    param._name = name;
    param._type = unargs__typeInt;
    param._req = false;
    param._desc = desc;
    param._def.i = def;
    param._dst = dst;

    return param;
}

unargs_Param unargs_intReq(
    const char *name, const char *desc, int *dst) {
    if (name != NULL) UNARGS_ASSERT(strlen(name) > 0);

    unargs_Param param;
    param._name = name;
    param._type = unargs__typeInt;
    param._req = true;
    param._desc = desc;
    param._dst = dst;

    return param;
}

unargs_Param unargs_unsigned(
    const char *name, const char *desc, unsigned def, unsigned *dst) {
    if (name != NULL) UNARGS_ASSERT(strlen(name) > 0);

    unargs_Param param;
    param._name = name;
    param._type = unargs__typeUnsigned;
    param._req = false;
    param._desc = desc;
    param._def.u = def;
    param._dst = dst;

    return param;
}

unargs_Param unargs_unsignedReq(
    const char *name, const char *desc, unsigned *dst) {
    if (name != NULL) UNARGS_ASSERT(strlen(name) > 0);

    unargs_Param param;
    param._name = name;
    param._type = unargs__typeUnsigned;
    param._req = true;
    param._desc = desc;
    param._dst = dst;

    return param;
}

unargs_Param unargs_long(
    const char *name, const char *desc, long def, long *dst) {
    if (name != NULL) UNARGS_ASSERT(strlen(name) > 0);

    unargs_Param param;
    param._name = name;
    param._type = unargs__typeLong;
    param._req = false;
    param._desc = desc;
    param._def.l = def;
    param._dst = dst;

    return param;
}

unargs_Param unargs_longReq(
    const char *name, const char *desc, long *dst) {
    if (name != NULL) UNARGS_ASSERT(strlen(name) > 0);

    unargs_Param param;
    param._name = name;
    param._type = unargs__typeLong;
    param._req = true;
    param._desc = desc;
    param._dst = dst;

    return param;
}

unargs_Param unargs_float(
    const char *name, const char *desc, float def, float *dst) {
    if (name != NULL) UNARGS_ASSERT(strlen(name) > 0);

    unargs_Param param;
    param._name = name;
    param._type = unargs__typeFloat;
    param._req = false;
    param._desc = desc;
    param._def.f = def;
    param._dst = dst;

    return param;
}

unargs_Param unargs_floatReq(
    const char *name, const char *desc, float *dst) {
    if (name != NULL) UNARGS_ASSERT(strlen(name) > 0);

    unargs_Param param;
    param._name = name;
    param._type = unargs__typeFloat;
    param._req = true;
    param._desc = desc;
    param._dst = dst;

    return param;
}

unargs_Param unargs_double(
    const char *name, const char *desc, double def, double *dst) {
    if (name != NULL) UNARGS_ASSERT(strlen(name) > 0);

    unargs_Param param;
    param._name = name;
    param._type = unargs__typeDouble;
    param._req = false;
    param._desc = desc;
    param._def.d = def;
    param._dst = dst;

    return param;
}

unargs_Param unargs_doubleReq(
    const char *name, const char *desc, double *dst) {
    if (name != NULL) UNARGS_ASSERT(strlen(name) > 0);

    unargs_Param param;
    param._name = name;
    param._type = unargs__typeDouble;
    param._req = true;
    param._desc = desc;
    param._dst = dst;

    return param;
}

unargs_Param unargs_string(
    const char *name, const char *desc, const char *def, const char **dst) {
    if (name != NULL) UNARGS_ASSERT(strlen(name) > 0);

    unargs_Param param;
    param._name = name;
    param._type = unargs__typeString;
    param._req = false;
    param._desc = desc;
    param._def.str = def;
    param._dst = dst;

    return param;
}

unargs_Param unargs_stringReq(
    const char *name, const char *desc, const char **dst) {
    if (name != NULL) UNARGS_ASSERT(strlen(name) > 0);

    unargs_Param param;
    param._name = name;
    param._type = unargs__typeString;
    param._req = true;
    param._desc = desc;
    param._dst = dst;

    return param;
}

bool unargs__paramIsOpt(const unargs_Param *param) {
    return param->_name != NULL;
}

bool unargs__paramIsPos(const unargs_Param *param) {
    return param->_name == NULL;
}

void unargs__printErrorPrefix(void) {
    UNARGS_PRINT_ERR_STR("Error: ");
}

void unargs__verifyParams(int len, const unargs_Param *params) {
    UNARGS_ASSERT(len >= 0);
    if (len > 0) UNARGS_ASSERT(params != NULL);

    bool posNonReqFound = false;
    for (int i = 0; i < len; ++i) {
        if (unargs__paramIsPos(&params[i])) {
            // Required positionals must preceed non-required ones.
            if (params[i]._req) UNARGS_ASSERT(!posNonReqFound);
            else posNonReqFound = true;
        }
    }

    for (int i = 0; i < len; ++i) {
        if (!unargs__paramIsOpt(&params[i])) continue;

        for (int j = i + 1; j < len; ++j) {
            if (!unargs__paramIsOpt(&params[j])) continue;

            // Names of optionals must be unique.
            UNARGS_ASSERT(strcmp(params[i]._name, params[j]._name) != 0);
        }
    }

    for (int i = 0; i < len; ++i) {
        for (int j = i + 1; j < len; ++j) {
            if (params[i]._dst != NULL && params[j]._dst != NULL) {
                // Value destinations must be unique, unless null.
                UNARGS_ASSERT(params[i]._dst != params[j]._dst);
            }
        }
    }
}

bool unargs__isOpt(const char *arg) {
    return arg[0] == '-' && arg[1] != '\0';
}

const char* unargs__optName(const char *arg) {
    return arg + 1;
}

int unargs__verifyArgs(int argc, char * const *argv) {
    if (argc < 1) {
        unargs__printErrorPrefix();
        UNARGS_PRINT_ERR_STR("At least one argument expected (program name).");
        UNARGS_PRINT_ERR_LN();
        return -1;
    }
    if (argv == NULL) {
        unargs__printErrorPrefix();
        UNARGS_PRINT_ERR_STR("argv must not be null.");
        UNARGS_PRINT_ERR_LN();
        return -1;
    }
    for (int i = 0; i < argc; ++i) {
        if (argv[i] == NULL) {
            unargs__printErrorPrefix();
            UNARGS_PRINT_ERR_STR("argv[");
            UNARGS_PRINT_ERR_INT(i);
            UNARGS_PRINT_ERR_STR("] is null.");
            UNARGS_PRINT_ERR_LN();
            return -1;
        }
        if (strlen(argv[i]) == 0) {
            unargs__printErrorPrefix();
            UNARGS_PRINT_ERR_STR("argv[");
            UNARGS_PRINT_ERR_INT(i);
            UNARGS_PRINT_ERR_STR("] is empty.");
            UNARGS_PRINT_ERR_LN();
            return -1;
        }
    }

    return 0;
}

bool unargs__optNameMatches(const char *arg, const unargs_Param *param) {
    return unargs__paramIsOpt(param) && strcmp(arg + 1, param->_name) == 0;
}

int unargs__argCnt(const unargs_Param *param) {
    if (unargs__paramIsOpt(param) && param->_type != unargs__typeBool) return 2;
    return 1;
}

void unargs__writeDef(unargs_Param *param) {
    if (param->_type == unargs__typeBool) {
        if (param->_dst != NULL) *(bool*)param->_dst = param->_def.b;
    } else if (param->_type == unargs__typeInt) {
        if (param->_dst != NULL) *(int*)param->_dst = param->_def.i;
    } else if (param->_type == unargs__typeUnsigned) {
        if (param->_dst != NULL) *(unsigned*)param->_dst = param->_def.u;
    } else if (param->_type == unargs__typeLong) {
        if (param->_dst != NULL) *(long*)param->_dst = param->_def.l;
    } else if (param->_type == unargs__typeFloat) {
        if (param->_dst != NULL) *(float*)param->_dst = param->_def.f;
    } else if (param->_type == unargs__typeDouble) {
        if (param->_dst != NULL) *(double*)param->_dst = param->_def.d;
    } else if (param->_type == unargs__typeString) {
        if (param->_dst != NULL) *(const char**)param->_dst = param->_def.str;
    } else {
        UNARGS_ASSERT(false);
    }
}

void unargs__printTypeOut(enum unargs__Type type) {
    if (type == unargs__typeInt) UNARGS_PRINT_OUT_STR("<int>");
    else if (type == unargs__typeUnsigned) UNARGS_PRINT_OUT_STR("<unsigned>");
    else if (type == unargs__typeLong) UNARGS_PRINT_OUT_STR("<long>");
    else if (type == unargs__typeFloat) UNARGS_PRINT_OUT_STR("<float>");
    else if (type == unargs__typeDouble) UNARGS_PRINT_OUT_STR("<double>");
    else if (type == unargs__typeString) UNARGS_PRINT_OUT_STR("<string>");
    else UNARGS_ASSERT(false);
}

void unargs__printTypeErr(enum unargs__Type type) {
    if (type == unargs__typeInt) UNARGS_PRINT_ERR_STR("<int>");
    else if (type == unargs__typeUnsigned) UNARGS_PRINT_ERR_STR("<unsigned>");
    else if (type == unargs__typeLong) UNARGS_PRINT_ERR_STR("<long>");
    else if (type == unargs__typeFloat) UNARGS_PRINT_ERR_STR("<float>");
    else if (type == unargs__typeDouble) UNARGS_PRINT_ERR_STR("<double>");
    else if (type == unargs__typeString) UNARGS_PRINT_ERR_STR("<string>");
    else UNARGS_ASSERT(false);
}

int unargs__parseLong(const char *str, long *l) {
    char *end;
    *l = strtol(str, &end, 0);
    if (*end != '\0') {
        unargs__printErrorPrefix();
        UNARGS_PRINT_ERR_STR("Could not parse integer value from '");
        UNARGS_PRINT_ERR_STR(str);
        UNARGS_PRINT_ERR_STR("'.");
        UNARGS_PRINT_ERR_LN();
        return -1;
    }

    return 0;
}

int unargs__parseUnsignedLong(const char *str, unsigned long *ul) {
    char *end;
    *ul = strtoul(str, &end, 0);
    if (*end != '\0') {
        unargs__printErrorPrefix();
        UNARGS_PRINT_ERR_STR(
            "Could not parse non-negative integer value from '");
        UNARGS_PRINT_ERR_STR(str);
        UNARGS_PRINT_ERR_STR("'.");
        UNARGS_PRINT_ERR_LN();
        return -1;
    }

    return 0;
}

int unargs__parseFloat(const char *str, float *f) {
    char *end;
    *f = strtof(str, &end);
    if (*end != '\0') {
        unargs__printErrorPrefix();
        UNARGS_PRINT_ERR_STR("Could not parse float value from '");
        UNARGS_PRINT_ERR_STR(str);
        UNARGS_PRINT_ERR_STR("'.");
        UNARGS_PRINT_ERR_LN();
        return -1;
    }

    return 0;
}

int unargs__parseDouble(const char *str, double *d) {
    char *end;
    *d = strtod(str, &end);
    if (*end != '\0') {
        unargs__printErrorPrefix();
        UNARGS_PRINT_ERR_STR("Could not parse double value from '");
        UNARGS_PRINT_ERR_STR(str);
        UNARGS_PRINT_ERR_STR("'.");
        UNARGS_PRINT_ERR_LN();
        return -1;
    }

    return 0;
}

int unargs__parseVal(const char *str, const unargs_Param *param) {
    if (param->_type == unargs__typeInt) {
        long l;
        if (unargs__parseLong(str, &l) < 0) return -1;

        int i = (int)l;
        if (i != l) {
            unargs__printErrorPrefix();
            UNARGS_PRINT_ERR_STR("Could not fit value ");
            UNARGS_PRINT_ERR_STR(str);
            UNARGS_PRINT_ERR_STR(" inside an int.");
            UNARGS_PRINT_ERR_LN();
            return -1;
        }

        if (param->_dst != NULL) *(int*)param->_dst = i;
    } else if (param->_type == unargs__typeUnsigned) {
        unsigned long ul;
        if (unargs__parseUnsignedLong(str, &ul) < 0) return -1;

        unsigned u = (unsigned)ul;
        if (u != ul) {
            unargs__printErrorPrefix();
            UNARGS_PRINT_ERR_STR("Could not fit value ");
            UNARGS_PRINT_ERR_STR(str);
            UNARGS_PRINT_ERR_STR(" inside an unsigned.");
            UNARGS_PRINT_ERR_LN();
            return -1;
        }

        if (param->_dst != NULL) *(unsigned*)param->_dst = u;
    } else if (param->_type == unargs__typeLong) {
        long l;
        if (unargs__parseLong(str, &l) < 0) return -1;

        if (param->_dst != NULL) *(long*)param->_dst = l;
    } else if (param->_type == unargs__typeFloat) {
        float f;
        if (unargs__parseFloat(str, &f) < 0) return -1;

        if (param->_dst != NULL) *(float*)param->_dst = f;
    } else if (param->_type == unargs__typeDouble) {
        double d;
        if (unargs__parseDouble(str, &d) < 0) return -1;

        if (param->_dst != NULL) *(double*)param->_dst = d;
    } else if (param->_type == unargs__typeString) {
        if (param->_dst != NULL) *(const char**)param->_dst = str;
    } else {
        UNARGS_ASSERT(false);
    }

    return 0;
}

int unargs__parseArgs(
    int argc, char * const *argv,
    int len, unargs_Param *params) {
    for (int p = 0; p < len; ++p) {
        params[p]._found = false;
        if (!params[p]._req) unargs__writeDef(&params[p]);
    }

    int nextPos = 0;
    while (nextPos < len && unargs__paramIsOpt(&params[nextPos])) {
        ++nextPos;
    }

    for (int a = 1; a < argc;) {
        if (unargs__isOpt(argv[a])) {
            unargs_Param *param = NULL;
            for (int p = 0; p < len && param == NULL; ++p) {
                if (unargs__optNameMatches(argv[a], &params[p])) {
                    param = &params[p];

                    if (param->_found) {
                        unargs__printErrorPrefix();
                        UNARGS_PRINT_ERR_STR("Option '");
                        UNARGS_PRINT_ERR_STR(param->_name);
                        UNARGS_PRINT_ERR_STR("' provided more than once.");
                        UNARGS_PRINT_ERR_LN();
                        return -1;
                    }

                    if (param->_type == unargs__typeBool) {
                        if (param->_dst != NULL) *(bool*)param->_dst = true;
                    } else {
                        if (a + 1 >= argc) {
                            unargs__printErrorPrefix();
                            UNARGS_PRINT_ERR_STR("Value not provided for '-");
                            UNARGS_PRINT_ERR_STR(param->_name);
                            UNARGS_PRINT_ERR_STR("'.");
                            UNARGS_PRINT_ERR_LN();
                            return -1;
                        }

                        if (unargs__parseVal(argv[a + 1], param) < 0) {
                            return -1;
                        }
                    }

                    param->_found = true;
                }
            }

            if (param == NULL) {
                unargs__printErrorPrefix();
                UNARGS_PRINT_ERR_STR("Found unknown option '-");
                UNARGS_PRINT_ERR_STR(unargs__optName(argv[a]));
                UNARGS_PRINT_ERR_STR("'.");
                UNARGS_PRINT_ERR_LN();
                return -1;
            }

            a += unargs__argCnt(param);
        } else {
            if (nextPos >= len) {
                unargs__printErrorPrefix();
                UNARGS_PRINT_ERR_STR("Found too many positional arguments,");
                UNARGS_PRINT_ERR_STR(" starting at '");
                UNARGS_PRINT_ERR_STR(argv[a]);
                UNARGS_PRINT_ERR_STR("'.");
                UNARGS_PRINT_ERR_LN();
                return -1;
            }

            if (unargs__parseVal(argv[a], &params[nextPos]) < 0) {
                return -1;
            }
            params[nextPos]._found = true;

            ++nextPos;
            while (nextPos < len && unargs__paramIsOpt(&params[nextPos])) {
                ++nextPos;
            }

            a += 1;
        }
    }

    for (int p = 0; p < len; ++p) {
        const unargs_Param *param = &params[p];

        if (param->_req && !param->_found) {
            unargs__printErrorPrefix();
            if (unargs__paramIsOpt(param)) {
                UNARGS_PRINT_ERR_STR("Required option '-");
                UNARGS_PRINT_ERR_STR(param->_name);
                UNARGS_PRINT_ERR_STR("' not given.");
            } else {
                UNARGS_PRINT_ERR_STR("Too few positional arguments provided.");
                UNARGS_PRINT_ERR_STR(" Next one should be of type ");
                unargs__printTypeErr(param->_type);
                UNARGS_PRINT_ERR_STR(".");
            }
            UNARGS_PRINT_ERR_LN();
            return -1;
        }
    }

    return 0;
}

int unargs_parse(
    int argc, char * const *argv,
    int len, unargs_Param *params) {
    unargs__verifyParams(len, params);
    if (unargs__verifyArgs(argc, argv) < 0) return -1;

    if (unargs__parseArgs(argc, argv, len, params) < 0) return -1;

    return 0;
}

void unargs__printDef(const unargs_Param *param) {
    if (param->_type == unargs__typeInt) {
        UNARGS_PRINT_OUT_INT(param->_def.i);
    } else if (param->_type == unargs__typeUnsigned) {
        UNARGS_PRINT_OUT_UNSIGNED(param->_def.u);
    } else if (param->_type == unargs__typeLong) {
        UNARGS_PRINT_OUT_LONG(param->_def.l);
    } else if (param->_type == unargs__typeFloat) {
        UNARGS_PRINT_OUT_FLOAT(param->_def.f);
    } else if (param->_type == unargs__typeDouble) {
        UNARGS_PRINT_OUT_DOUBLE(param->_def.d);
    } else if (param->_type == unargs__typeString) {
        UNARGS_PRINT_OUT_STR(param->_def.str);
    } else {
        UNARGS_ASSERT(false);
    }
}

void unargs__printUsage(
    const char *program, int len, const unargs_Param *params) {
    UNARGS_PRINT_OUT_STR("Usage: ");

    if (program != NULL) UNARGS_PRINT_OUT_STR(program);

    for (int i = 0; i < len; ++i) {
        const unargs_Param *param = &params[i];

        if (unargs__paramIsPos(param) && param->_req) {
            UNARGS_PRINT_OUT_STR(" ");
            unargs__printTypeOut(param->_type);
        }
    }

    for (int i = 0; i < len; ++i) {
        const unargs_Param *param = &params[i];

        if (unargs__paramIsOpt(param) && param->_req) {
            UNARGS_PRINT_OUT_STR(" -");
            UNARGS_PRINT_OUT_STR(param->_name);
            UNARGS_PRINT_OUT_STR(" ");
            unargs__printTypeOut(param->_type);
        }
    }

    bool hasNonReqPos = false;
    for (int i = 0; i < len; ++i) {
        const unargs_Param *param = &params[i];

        if (unargs__paramIsPos(param) && !param->_req) {
            hasNonReqPos = true;
            break;
        }
    }
    if (hasNonReqPos) UNARGS_PRINT_OUT_STR(" [positionals]");

    bool hasNonReqOpt = false;
    for (int i = 0; i < len; ++i) {
        const unargs_Param *param = &params[i];

        if (unargs__paramIsOpt(param) && !param->_req) {
            hasNonReqOpt = true;
            break;
        }
    }
    if (hasNonReqOpt) UNARGS_PRINT_OUT_STR(" [options]");

    UNARGS_PRINT_OUT_LN();
}

void unargs__printRequired(const unargs_Param *param) {
    if (param->_req) {
        UNARGS_PRINT_OUT_TAB();
        UNARGS_PRINT_OUT_TAB();
        UNARGS_PRINT_OUT_STR("(required)");
        UNARGS_PRINT_OUT_LN();
    }
}

void unargs__printDescription(const unargs_Param *param) {
    if (param->_desc != NULL) {
        UNARGS_PRINT_OUT_TAB();
        UNARGS_PRINT_OUT_TAB();
        UNARGS_PRINT_OUT_STR(param->_desc);
        UNARGS_PRINT_OUT_LN();
    }
}

void unargs__printDefault(const unargs_Param *param) {
    if (!param->_req && param->_type != unargs__typeBool) {
        UNARGS_PRINT_OUT_TAB();
        UNARGS_PRINT_OUT_TAB();
        UNARGS_PRINT_OUT_STR("Default: ");
        unargs__printDef(param);
        UNARGS_PRINT_OUT_LN();
    }
}

void unargs__printPositionals(int len, const unargs_Param *params) {
    bool hasPos = false;
    for (int i = 0; i < len; ++i) {
        if (unargs__paramIsPos(&params[i])) {
            hasPos = true;
            break;
        }
    }
    if (!hasPos) return;

    UNARGS_PRINT_OUT_STR("Positionals:");
    UNARGS_PRINT_OUT_LN();
    for (int i = 0; i < len; ++i) {
        const unargs_Param *param = &params[i];
        if (!unargs__paramIsPos(param)) continue;

        UNARGS_PRINT_OUT_TAB();
        unargs__printTypeOut(param->_type);
        UNARGS_PRINT_OUT_LN();

        unargs__printRequired(param);
        unargs__printDescription(param);
        unargs__printDefault(param);
    }
}

void unargs__printOptions(int len, const unargs_Param *params) {
    bool hasOpt = false;
    for (int i = 0; i < len; ++i) {
        if (unargs__paramIsOpt(&params[i])) {
            hasOpt = true;
            break;
        }
    }
    if (!hasOpt) return;

    UNARGS_PRINT_OUT_STR("Options:");
    UNARGS_PRINT_OUT_LN();
    for (int i = 0; i < len; ++i) {
        const unargs_Param *param = &params[i];
        if (!unargs__paramIsOpt(param)) continue;

        UNARGS_PRINT_OUT_TAB();
        UNARGS_PRINT_OUT_STR("-");
        UNARGS_PRINT_OUT_STR(param->_name);
        if (param->_type != unargs__typeBool) {
            UNARGS_PRINT_OUT_STR(" ");
            unargs__printTypeOut(param->_type);
        }
        UNARGS_PRINT_OUT_LN();

        unargs__printRequired(param);
        unargs__printDescription(param);
        unargs__printDefault(param);
    }
}

void unargs_help(const char *program, int len, const unargs_Param *params) {
    unargs__verifyParams(len, params);

    unargs__printUsage(program, len, params);
    unargs__printPositionals(len, params);
    unargs__printOptions(len, params);
}

#endif // UNARGS_IMPLEMENTATION
