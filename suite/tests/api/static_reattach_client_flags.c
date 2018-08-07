/* **********************************************************
 * Copyright (c) 2016-2017 Google, Inc.  All rights reserved.
 * **********************************************************/

/*
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * * Neither the name of Google, Inc. nor the names of its contributors may be
 *   used to endorse or promote products derived from this software without
 *   specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL GOOGLE, INC. OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 */

#include "configure.h"
#include "dr_api.h"
#include "tools.h"
#include <math.h>

/* A test to verify that flags are appropriately piped through to client
 * libraries for static reattach, verifying that the lazy-loading logic is
 * correctly reset for reattach.
 */

// last_argv is updated in dr_client_main with a copy of the 1th argv passed to
// that function.
static const char *last_argv;

DR_EXPORT void
dr_client_main(client_id_t id, int argc, const char *argv[])
{
    print("in dr_client_main with argc=%d\n", argc);
    if (argc != 2) {
        print("ERROR: want only 2 argc!\n");
        return;
    };
    last_argv = strdup(argv[1]);
}

typedef struct {
    const char* input_dynamorio_options;

    const char* want_argv;
} test_arg_t;

#define TEST_ARG_COUNT 3
const test_arg_t test_args[TEST_ARG_COUNT] = {
    {
        .input_dynamorio_options = "-client_lib ';;a'",
        .want_argv = "a",
    }, {
        .input_dynamorio_options = "-client_lib ';;b'",
        .want_argv = "b",
    }, {
        .input_dynamorio_options = "-client_lib ';;c'",
        .want_argv = "c",
    },
};

int
main(int argc, const char *argv[])
{
    for (int i = 0; i < TEST_ARG_COUNT; i++) {
        setenv("DYNAMORIO_OPTIONS", test_args[i].input_dynamorio_options, 1);
        dr_app_setup();
        dr_app_start();
        dr_app_stop_and_cleanup();

        if (last_argv == NULL) {
            print("ERROR: last_argv not set by dr_client_main");
            return 1;
        }
        const char* want_argv = test_args[i].want_argv;
        if (0 != strcmp(last_argv, want_argv)) {
            print("ERROR: last_argv doesn't match want_argv: "
                  "got |%s|, want |%s|", last_argv, want_argv);
            return 1;
        }
        print("Found the appropriate argv\n");
        free((void*)last_argv);
        last_argv = NULL;
    }

    print("all done\n");
    return 0;
}
