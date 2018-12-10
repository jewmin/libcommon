#include "get_opt.h"

int arg_counter = 1;
char jc_optarg[514];
int jc_getopt_long_only(int ___argc, char * const * ___argv, const char * __shortopts, const struct jc_option * __longopts, int * __longind) {
    // burlex todo: handle the shortops, at the moment it only works with longopts.

    // No arguments
    if (1 == ___argc || arg_counter == ___argc) {
        return -1;
    }

    const char * opt = ___argv[arg_counter];

    // if we're not an option, return an error
    if (strncasecmp(opt, "--", 2) != 0) {
        return 1;
    } else {
        opt += 2;
    }

    // parse argument list
    for (int i = 0; __longopts[i].name != nullptr; ++i) {
        if (strncasecmp(__longopts[i].name, opt, strlen(__longopts[i].name)) == 0) {
            // woot, found a valid argument =)
            char * par = nullptr;
            if ((arg_counter + 1) != ___argc) {
                // grab the parameter from the next argument (if its not another argument)
                if (strncasecmp(___argv[arg_counter + 1], "--", 2) != 0) {
                    ++arg_counter; // Trash this next argument, we won't be needing it.
                    par = ___argv[arg_counter];
                }
            }

            // increment the argument for next time
            ++arg_counter;

            // determine action based on type
            if (jc_required_argument == __longopts[i].has_arg) {
                if (!par) {
                    return 1;
                }

                // parameter missing and its a required parameter option
                if (__longopts[i].flag) {
                    *__longopts[i].flag = atoi(par);
                    return 0;
                }
            }

            // store argument in optarg
            if (par) {
                STRNCPY_S(jc_optarg, par);
            }

            if (__longopts[i].flag) {
                // this is a variable, we have to set it if this argument is found.
                *__longopts[i].flag = 1;
                return 0;
            } else {
                if (__longopts[i].val == -1 || !par) {
                    return 1;
                }

                return __longopts[i].val;
            }

            break;
        }
    }

    // invalid argument
    return 1;
}