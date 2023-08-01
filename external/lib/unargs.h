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

/*
This is a single-header-file library for parsing main arguments (argc and argv).
It does NOT follow Unix conventions! If that is what you want, this is not the
library to use.

Include this file like this:

    #define UNARGS_IMPLEMENTATION
    #include "unargs.h"

in ONE of your source files to include the implementation. You can then include
it without the define in other files and not get linker errors.

To parse argc and argv, first create an array of unargs_Param objects using
unargs_<type> and unargs_<type>Req functions, then pass this array to
unargs_parse.

It should look like this:

    unargs_Param params[] = {
        // required option '-i1'
        unargs_intReq("i1", "Description.", &i1),

        // non-required option '-i2', default value is 100
        unargs_int("i2", "Description.", 100, &i2),

        // required positional
        unargs_intReq(NULL, "Description.", &i3),

        // non-required positional, default value is 200
        unargs_int(NULL, "Description.", 200, &i4),

        // bools are specific:
        // they are always non-required,
        // they cannot be positionals,
        // and their default value is always false
        unargs_bool("b", "Description.", &b),
    };
    unargs_parse(argc, argv, sizeof(params) / sizeof(*params), params);

Valid ways to call the program would then be:

    # pass in all values, set b to true
    main -i1 1111 -i2 2222 3333 4444 -b

    # pass in all values, leave b as false
    main -i1 1111 -i2 2222 3333 4444

    # pass in only required values
    main -i1 1111 3333

    # pass in both options and only the required positional
    main -i1 1111 -i2 2222 3333

    # pass in both positionals and only the required option
    main -i1 1111 3333 4444

    # ordering of positionals matters
    # and options must be passed in as '-name val',
    # otherwise any order is fine
    main -i2 2222 -i1 1111 3333 4444 -b
    main 3333 -b 4444 -i1 1111 -i2 2222
    main -b 3333 -i2 2222 -i1 1111 4444

unargs_parse and unargs_help return unargs_ok (0) on success, non-zero values on
failure.

Some terminology:
    parameter - part of your interface;
    argument - value being passed through argv.

Parameters can be options or positionals. Options are passed in as '-name val'.
Positionals have no names and are passed in with just the value.

Parameters can be required or non-required. unargs_parse checks that all
required arguments have been passed in. Non-required parameters will be given a
default value if not passed in.

Bools are an exception:
    1) they cannot be positionals;
    2) they are always non-required with the default of false;
    3) they are set to true by passing in '-name' (no value after).

unargs_help prints usage instructions on the standard output:

    unargs_help("main", sizeof(params) / sizeof(*params), params);

Both functions use fprintf to print to stdout and stderr. You can override this
by defining print macros (before including this header):

    #define UNARGS_PRINT_OUT_<TYPE>(x) ...
    #define UNARGS_PRINT_OUT_TAB(x) ... # tab character
    #define UNARGS_PRINT_OUT_LN(x) ... # new line

    #define UNARGS_PRINT_ERR_<TYPE>(x) ...
    #define UNARGS_PRINT_ERR_TAB(x) ... # tab character
    #define UNARGS_PRINT_ERR_LN(x) ... # new line

You can also define UNARGS_ASSERT(x) if you don't want unargs to use C's assert.
*/

#ifndef INCLUDE_UNARGS_H
#define INCLUDE_UNARGS_H

#include <stdbool.h>

enum {
    // Call was successful.
    unargs_ok = 0,
    // There was an error due to user arguments.
    unargs_err_args = -1,
    // There was an error in parameters.
    unargs_err_params = -2
};

// Type that holds parameter properties.
typedef struct unargs_Param unargs_Param;

/**
 * Specify a bool parameter. Bool parameters are always non-required options
 * with the default value of false. They are passed in as \c -name without any
 * values after.
 *
 * \param name Name of the option. Defines how it will be passed in. Must not be
 * null nor an empty string.
 *
 * \param desc Description for the parameter to be displayed by \c unargs_help.
 *
 * \param dst Location where the parsed or default value will be written to.
 *
 * \return The created parameter. You can pass an array of these to
 * \c unargs_parse or \c unargs_help.
 */
unargs_Param unargs_bool(
    const char *name, const char *desc, bool *dst);

/**
 * Specify a non-required int parameter with a default value. The parameter will
 * be an option if \c name is not null, or positional otherwise.
 *
 * \param name Name of the parameter. Defines how it will be passed in. If
 * \c name is not null, this parameter will be an option. Otherwise, it will be
 * positional. Must not be an empty string.
 *
 * \param desc Description for the parameter to be displayed by \c unargs_help.
 *
 * \param def The default value to be assigned in case argument is not provided.
 *
 * \param dst Location where the parsed or default value will be written to.
 *
 * \return The created parameter. You can pass an array of these to
 * \c unargs_parse or \c unargs_help.
 */
unargs_Param unargs_int(
    const char *name, const char *desc, int def, int *dst);

/**
 * Specify a required int parameter. The parameter will be an option if \c name
 * is not null, or positional otherwise.
 *
 * \param name Name of the parameter. Defines how it will be passed in. If
 * \c name is not null, this parameter will be an option. Otherwise, it will be
 * positional. Must not be an empty string.
 *
 * \param desc Description for the parameter to be displayed by \c unargs_help.
 *
 * \param dst Location where the parsed or default value will be written to.
 *
 * \return The created parameter. You can pass an array of these to
 * \c unargs_parse or \c unargs_help.
 */
unargs_Param unargs_intReq(
    const char *name, const char *desc, int *dst);

/**
 * Specify a non-required unsigned parameter with a default value. The parameter
 * will be an option if \c name is not null, or positional otherwise.
 *
 * \param name Name of the parameter. Defines how it will be passed in. If
 * \c name is not null, this parameter will be an option. Otherwise, it will be
 * positional. Must not be an empty string.
 *
 * \param desc Description for the parameter to be displayed by \c unargs_help.
 *
 * \param def The default value to be assigned in case argument is not provided.
 *
 * \param dst Location where the parsed or default value will be written to.
 *
 * \return The created parameter. You can pass an array of these to
 * \c unargs_parse or \c unargs_help.
 */
unargs_Param unargs_unsigned(
    const char *name, const char *desc, unsigned def, unsigned *dst);

/**
 * Specify a required unsigned parameter. The parameter will be an option if
 * \c name is not null, or positional otherwise.
 *
 * \param name Name of the parameter. Defines how it will be passed in. If
 * \c name is not null, this parameter will be an option. Otherwise, it will be
 * positional. Must not be an empty string.
 *
 * \param desc Description for the parameter to be displayed by \c unargs_help.
 *
 * \param dst Location where the parsed or default value will be written to.
 *
 * \return The created parameter. You can pass an array of these to
 * \c unargs_parse or \c unargs_help.
 */
unargs_Param unargs_unsignedReq(
    const char *name, const char *desc, unsigned *dst);

/**
 * Specify a non-required float parameter with a default value. The parameter
 * will be an option if \c name is not null, or positional otherwise.
 *
 * \param name Name of the parameter. Defines how it will be passed in. If
 * \c name is not null, this parameter will be an option. Otherwise, it will be
 * positional. Must not be an empty string.
 *
 * \param desc Description for the parameter to be displayed by \c unargs_help.
 *
 * \param def The default value to be assigned in case argument is not provided.
 *
 * \param dst Location where the parsed or default value will be written to.
 *
 * \return The created parameter. You can pass an array of these to
 * \c unargs_parse or \c unargs_help.
 */
unargs_Param unargs_float(
    const char *name, const char *desc, float def, float *dst);

/**
 * Specify a required float parameter. The parameter will be an option if
 * \c name is not null, or positional otherwise.
 *
 * \param name Name of the parameter. Defines how it will be passed in. If
 * \c name is not null, this parameter will be an option. Otherwise, it will be
 * positional. Must not be an empty string.
 *
 * \param desc Description for the parameter to be displayed by \c unargs_help.
 *
 * \param dst Location where the parsed or default value will be written to.
 *
 * \return The created parameter. You can pass an array of these to
 * \c unargs_parse or \c unargs_help.
 */
unargs_Param unargs_floatReq(
    const char *name, const char *desc, float *dst);

/**
 * Specify a non-required string parameter with a default value. The parameter
 * will be an option if \c name is not null, or positional otherwise.
 *
 * \param name Name of the parameter. Defines how it will be passed in. If
 * \c name is not null, this parameter will be an option. Otherwise, it will be
 * positional. Must not be an empty string.
 *
 * \param desc Description for the parameter to be displayed by \c unargs_help.
 *
 * \param def The default value to be assigned in case argument is not provided.
 * This string pointer will be assigned to the destination - be careful about
 * lifetime implications of this.
 *
 * \param dst Location where the parsed or default value will be written to.
 *
 * \return The created parameter. You can pass an array of these to
 * \c unargs_parse or \c unargs_help.
 */
unargs_Param unargs_string(
    const char *name, const char *desc, const char *def, const char **dst);

/**
 * Specify a required string parameter. The parameter will be an option if
 * \c name is not null, or positional otherwise.
 *
 * \param name Name of the parameter. Defines how it will be passed in. If
 * \c name is not null, this parameter will be an option. Otherwise, it will be
 * positional. Must not be an empty string.
 *
 * \param desc Description for the parameter to be displayed by \c unargs_help.
 *
 * \param dst Location where the parsed or default value will be written to.
 *
 * \return The created parameter. You can pass an array of these to
 * \c unargs_parse or \c unargs_help.
 */
unargs_Param unargs_stringReq(
    const char *name, const char *desc, const char **dst);

/**
 * Parses \c main function's arguments and assigns them (or the default values)
 * to the destinations of parameters.
 *
 * It will verify that parameters and arguments are well-formed. If not, an
 * error code is returned. Error messages will be printed out using
 * UNARGS_PRINT_ERR_* macros.
 *
 * \param argc Number of arguments in \c argv. You probably want to pass the
 * first argument to \c main here. Must be at least 1.
 *
 * \param argv Array of arguments from the command-line. You probably want to
 * pass the second argument to \c main here. Must not be null. All elements must
 * not be null.
 *
 * \param len Number of parameters in \c params. Must be non-negative.
 *
 * \param params Array of arguments created with \c unargs_<type> and
 * \c unargs_<type>Req functions. If \c len is positive, must not be null.
 *
 * \returns On success, returns \c unargs_ok (zero). On failure, returns a
 * non-zero value: \c unargs_err_args if user arguments are not well-formed,
 * \c unargs_err_params if parameters in \c params are not well-formed.
 */
int unargs_parse(
    int argc, char * const *argv,
    int len, unargs_Param *params);

/**
 * Prints a helpful text on how to use the program with the specified
 * parameters. Uses UNARGS_PRINT_OUT_* macros.
 *
 * It will verify that parameters are well-formed. If not, an error code is
 * returned. Error messages will be printed out using UNARGS_PRINT_ERR_* macros.
 *
 * \param program Name of your program to be listed as the zeroth argument.
 * \c main function's zeroth argument \c argv[0] usually corresponds to this in
 * some way.
 *
 * \param len Number of parameters in \c params. Must be non-negative.
 *
 * \param params Array of arguments created with \c unargs_<type> and
 * \c unargs_<type>Req functions. If \c len is positive, must not be null.
 *
 * \returns On success, returns \c unargs_ok (zero). On failure, returns a
 * non-zero value - \c unargs_err_params if parameters in \c params are not
 * well-formed.
 */
int unargs_help(
    const char *program,
    int len, const unargs_Param *params);

// Below are implementation details.

enum unargs__Type {
    unargs__typeBool,
    unargs__typeInt,
    unargs__typeUnsigned,
    unargs__typeFloat,
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
        float f;
        const char *str;
    } _def;

    void *_dst;

    bool _found;
};

#endif // INCLUDE_UNARGS_H

// IMPLEMENTATION

#ifdef UNARGS_IMPLEMENTATION

#include <errno.h>
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

#if !defined(UNARGS_PRINT_OUT_FLOAT)
#include <stdio.h>
#define UNARGS_PRINT_OUT_FLOAT(x) fprintf(stdout, "%f", x)
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

#if !defined(UNARGS_PRINT_ERR_FLOAT)
#include <stdio.h>
#define UNARGS_PRINT_ERR_FLOAT(x) fprintf(stderr, "%f", x)
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
    unargs_Param param;
    param._name = name;
    param._type = unargs__typeUnsigned;
    param._req = true;
    param._desc = desc;
    param._dst = dst;

    return param;
}

unargs_Param unargs_float(
    const char *name, const char *desc, float def, float *dst) {
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
    unargs_Param param;
    param._name = name;
    param._type = unargs__typeFloat;
    param._req = true;
    param._desc = desc;
    param._dst = dst;

    return param;
}

unargs_Param unargs_string(
    const char *name, const char *desc, const char *def, const char **dst) {
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

void unargs__printErrorParamsPrefix(void) {
    UNARGS_PRINT_ERR_STR("Parameter error: ");
}

void unargs__printErrorArgsPrefix(void) {
    UNARGS_PRINT_ERR_STR("Error: ");
}

int unargs__verifyParams(int len, const unargs_Param *params) {
    if (len < 0) {
        unargs__printErrorParamsPrefix();
        UNARGS_PRINT_ERR_STR("Parameter length must be non-negative, but is ");
        UNARGS_PRINT_ERR_INT(len);
        UNARGS_PRINT_ERR_STR(".");
        UNARGS_PRINT_ERR_LN();

        return unargs_err_params;
    }
    if (len > 0 && params == NULL) {
        unargs__printErrorParamsPrefix();
        UNARGS_PRINT_ERR_STR(
            "Parameter array must not be null when length is non-zero.");
        UNARGS_PRINT_ERR_LN();

        return unargs_err_params;
    }

    for (int i = 0; i < len; ++i) {
        const unargs_Param *param = &params[i];

        if (param->_type == unargs__typeBool) {
            if (param->_name == NULL || strlen(param->_name) == 0) {
                unargs__printErrorParamsPrefix();
                UNARGS_PRINT_ERR_STR(
                    "Bool parameter names must not be null or empty.");
                UNARGS_PRINT_ERR_LN();

                return unargs_err_params;
            }
        } else {
            if (param->_name != NULL && strlen(param->_name) == 0) {
                unargs__printErrorParamsPrefix();
                UNARGS_PRINT_ERR_STR(
                    "Parameter names must not be empty."
                    " To make a parameter positional,"
                    " declare its name as null.");
                UNARGS_PRINT_ERR_LN();

                return unargs_err_params;
            }
        }
    }

    bool posNonReqFound = false;
    for (int i = 0; i < len; ++i) {
        if (unargs__paramIsPos(&params[i])) {
            if (params[i]._req) {
                if (posNonReqFound) {
                    unargs__printErrorParamsPrefix();
                    UNARGS_PRINT_ERR_STR(
                        "Required positionals must preceed non-required ones.");
                    UNARGS_PRINT_ERR_LN();

                    return unargs_err_params;
                }
            } else {
                posNonReqFound = true;
            }
        }
    }

    for (int i = 0; i < len; ++i) {
        if (!unargs__paramIsOpt(&params[i])) continue;

        for (int j = i + 1; j < len; ++j) {
            if (!unargs__paramIsOpt(&params[j])) continue;

            if (strcmp(params[i]._name, params[j]._name) == 0) {
                unargs__printErrorParamsPrefix();
                UNARGS_PRINT_ERR_STR("Names of options must be unique.");
                UNARGS_PRINT_ERR_STR(" Option ");
                UNARGS_PRINT_ERR_STR(params[i]._name);
                UNARGS_PRINT_ERR_STR(" appears multiple times.");
                UNARGS_PRINT_ERR_LN();

                return unargs_err_params;
            }
        }
    }

    for (int i = 0; i < len; ++i) {
        for (int j = i + 1; j < len; ++j) {
            if (params[i]._dst != NULL && params[i]._dst == params[j]._dst) {
                unargs__printErrorParamsPrefix();
                UNARGS_PRINT_ERR_STR(
                    "Value destinations must be unique (unless null).");
                UNARGS_PRINT_ERR_LN();

                return unargs_err_params;
            }
        }
    }

    return unargs_ok;
}

bool unargs__isOpt(const char *arg) {
    return arg[0] == '-' && arg[1] != '\0';
}

const char* unargs__optName(const char *arg) {
    return arg + 1;
}

int unargs__verifyArgs(int argc, char * const *argv) {
    if (argc < 1) {
        unargs__printErrorArgsPrefix();
        UNARGS_PRINT_ERR_STR(
            "At least one argument expected (the program name).");
        UNARGS_PRINT_ERR_LN();

        return unargs_err_args;
    }
    if (argv == NULL) {
        unargs__printErrorArgsPrefix();
        UNARGS_PRINT_ERR_STR("argv must not be null.");
        UNARGS_PRINT_ERR_LN();

        return unargs_err_args;
    }
    for (int i = 0; i < argc; ++i) {
        if (argv[i] == NULL) {
            unargs__printErrorArgsPrefix();
            UNARGS_PRINT_ERR_STR("argv[");
            UNARGS_PRINT_ERR_INT(i);
            UNARGS_PRINT_ERR_STR("] is null.");
            UNARGS_PRINT_ERR_LN();

            return unargs_err_args;
        }
        if (strlen(argv[i]) == 0) {
            unargs__printErrorArgsPrefix();
            UNARGS_PRINT_ERR_STR("argv[");
            UNARGS_PRINT_ERR_INT(i);
            UNARGS_PRINT_ERR_STR("] is empty.");
            UNARGS_PRINT_ERR_LN();

            return unargs_err_args;
        }
    }

    return unargs_ok;
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
    } else if (param->_type == unargs__typeFloat) {
        if (param->_dst != NULL) *(float*)param->_dst = param->_def.f;
    } else if (param->_type == unargs__typeString) {
        if (param->_dst != NULL) *(const char**)param->_dst = param->_def.str;
    } else {
        UNARGS_ASSERT(false);
    }
}

void unargs__printTypeOut(enum unargs__Type type) {
    if (type == unargs__typeInt) UNARGS_PRINT_OUT_STR("<int>");
    else if (type == unargs__typeUnsigned) UNARGS_PRINT_OUT_STR("<unsigned>");
    else if (type == unargs__typeFloat) UNARGS_PRINT_OUT_STR("<float>");
    else if (type == unargs__typeString) UNARGS_PRINT_OUT_STR("<string>");
    else UNARGS_ASSERT(false);
}

void unargs__printTypeErr(enum unargs__Type type) {
    if (type == unargs__typeInt) UNARGS_PRINT_ERR_STR("<int>");
    else if (type == unargs__typeUnsigned) UNARGS_PRINT_ERR_STR("<unsigned>");
    else if (type == unargs__typeFloat) UNARGS_PRINT_ERR_STR("<float>");
    else if (type == unargs__typeString) UNARGS_PRINT_ERR_STR("<string>");
    else UNARGS_ASSERT(false);
}

int unargs__parseLong(const char *arg, long *l) {
    char *end;
    errno = 0;
    *l = strtol(arg, &end, 0);
    if (errno != 0 || *end != '\0') {
        unargs__printErrorArgsPrefix();
        UNARGS_PRINT_ERR_STR("Could not parse integer value from '");
        UNARGS_PRINT_ERR_STR(arg);
        UNARGS_PRINT_ERR_STR("'.");
        UNARGS_PRINT_ERR_LN();

        return unargs_err_args;
    }

    return unargs_ok;
}

int unargs__parseUnsignedLong(const char *arg, unsigned long *ul) {
    char *end;
    errno = 0;
    *ul = strtoul(arg, &end, 0);
    if (errno != 0 || *end != '\0') {
        unargs__printErrorArgsPrefix();
        UNARGS_PRINT_ERR_STR(
            "Could not parse non-negative integer value from '");
        UNARGS_PRINT_ERR_STR(arg);
        UNARGS_PRINT_ERR_STR("'.");
        UNARGS_PRINT_ERR_LN();

        return unargs_err_args;
    }

    return unargs_ok;
}

int unargs__parseFloat(const char *arg, float *f) {
    char *end;
    errno = 0;
    *f = strtof(arg, &end);
    if (errno != 0 || *end != '\0') {
        unargs__printErrorArgsPrefix();
        UNARGS_PRINT_ERR_STR("Could not parse float value from '");
        UNARGS_PRINT_ERR_STR(arg);
        UNARGS_PRINT_ERR_STR("'.");
        UNARGS_PRINT_ERR_LN();

        return unargs_err_args;
    }

    return unargs_ok;
}

int unargs__parseVal(const char *str, const unargs_Param *param) {
    if (param->_type == unargs__typeInt) {
        long l;
        if (unargs__parseLong(str, &l) != unargs_ok) {
            return unargs_err_args;
        }

        int i = (int)l;
        if (i != l) {
            unargs__printErrorArgsPrefix();
            UNARGS_PRINT_ERR_STR("Could not fit value ");
            UNARGS_PRINT_ERR_STR(str);
            UNARGS_PRINT_ERR_STR(" inside an int.");
            UNARGS_PRINT_ERR_LN();

            return unargs_err_args;
        }

        if (param->_dst != NULL) *(int*)param->_dst = i;
    } else if (param->_type == unargs__typeUnsigned) {
        unsigned long ul;
        if (unargs__parseUnsignedLong(str, &ul) != unargs_ok) {
            return unargs_err_args;
        }

        unsigned u = (unsigned)ul;
        if (u != ul) {
            unargs__printErrorArgsPrefix();
            UNARGS_PRINT_ERR_STR("Could not fit value ");
            UNARGS_PRINT_ERR_STR(str);
            UNARGS_PRINT_ERR_STR(" inside an unsigned.");
            UNARGS_PRINT_ERR_LN();

            return unargs_err_args;
        }

        if (param->_dst != NULL) *(unsigned*)param->_dst = u;
    } else if (param->_type == unargs__typeFloat) {
        float f;
        if (unargs__parseFloat(str, &f) != unargs_ok) {
            return unargs_err_args;
        }

        if (param->_dst != NULL) *(float*)param->_dst = f;
    } else if (param->_type == unargs__typeString) {
        if (param->_dst != NULL) *(const char**)param->_dst = str;
    } else {
        UNARGS_ASSERT(false);
    }

    return unargs_ok;
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
                        unargs__printErrorArgsPrefix();
                        UNARGS_PRINT_ERR_STR("Option '");
                        UNARGS_PRINT_ERR_STR(param->_name);
                        UNARGS_PRINT_ERR_STR("' provided more than once.");
                        UNARGS_PRINT_ERR_LN();

                        return unargs_err_args;
                    }

                    if (param->_type == unargs__typeBool) {
                        if (param->_dst != NULL) *(bool*)param->_dst = true;
                    } else {
                        if (a + 1 >= argc) {
                            unargs__printErrorArgsPrefix();
                            UNARGS_PRINT_ERR_STR("Value not provided for '-");
                            UNARGS_PRINT_ERR_STR(param->_name);
                            UNARGS_PRINT_ERR_STR("'.");
                            UNARGS_PRINT_ERR_LN();

                            return unargs_err_args;
                        }

                        if (unargs__parseVal(argv[a + 1], param) != unargs_ok) {
                            return unargs_err_args;
                        }
                    }

                    param->_found = true;
                }
            }

            if (param == NULL) {
                unargs__printErrorArgsPrefix();
                UNARGS_PRINT_ERR_STR("Found unknown option '-");
                UNARGS_PRINT_ERR_STR(unargs__optName(argv[a]));
                UNARGS_PRINT_ERR_STR("'.");
                UNARGS_PRINT_ERR_LN();

                return unargs_err_args;
            }

            a += unargs__argCnt(param);
        } else {
            if (nextPos >= len) {
                unargs__printErrorArgsPrefix();
                UNARGS_PRINT_ERR_STR("Found too many positional arguments,");
                UNARGS_PRINT_ERR_STR(" starting at '");
                UNARGS_PRINT_ERR_STR(argv[a]);
                UNARGS_PRINT_ERR_STR("'.");
                UNARGS_PRINT_ERR_LN();

                return unargs_err_args;
            }

            if (unargs__parseVal(argv[a], &params[nextPos]) != unargs_ok) {
                return unargs_err_args;
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
            unargs__printErrorArgsPrefix();
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

            return unargs_err_args;
        }
    }

    return unargs_ok;
}

int unargs_parse(
    int argc, char * const *argv,
    int len, unargs_Param *params) {
    if (unargs__verifyParams(len, params) != unargs_ok) {
        return unargs_err_params;
    }
    if (unargs__verifyArgs(argc, argv) != unargs_ok) {
        return unargs_err_args;
    }

    if (unargs__parseArgs(argc, argv, len, params) != unargs_ok) {
        return unargs_err_args;
    }

    return unargs_ok;
}

void unargs__printDef(const unargs_Param *param) {
    if (param->_type == unargs__typeInt) {
        UNARGS_PRINT_OUT_INT(param->_def.i);
    } else if (param->_type == unargs__typeUnsigned) {
        UNARGS_PRINT_OUT_UNSIGNED(param->_def.u);
    } else if (param->_type == unargs__typeFloat) {
        UNARGS_PRINT_OUT_FLOAT(param->_def.f);
    } else if (param->_type == unargs__typeString) {
        if (param->_def.str == NULL) {
            UNARGS_PRINT_OUT_STR("<null>");
        } else {
            UNARGS_PRINT_OUT_STR("\"");
            UNARGS_PRINT_OUT_STR(param->_def.str);
            UNARGS_PRINT_OUT_STR("\"");
        }
    } else {
        UNARGS_ASSERT(false);
    }
}

void unargs__printUsage(
    const char *program, int len, const unargs_Param *params) {
    UNARGS_PRINT_OUT_STR("Usage:");

    if (program != NULL) {
        UNARGS_PRINT_OUT_STR(" ");
        UNARGS_PRINT_OUT_STR(program);
    }

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

int unargs_help(
    const char *program,
    int len, const unargs_Param *params) {
    if (unargs__verifyParams(len, params) != unargs_ok) {
        return unargs_err_params;
    }

    unargs__printUsage(program, len, params);
    unargs__printPositionals(len, params);
    unargs__printOptions(len, params);

    return unargs_ok;
}

#endif // UNARGS_IMPLEMENTATION
