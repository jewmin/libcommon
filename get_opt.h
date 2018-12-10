#ifndef __LIBCOMMON_GET_OPT_H__
#define __LIBCOMMON_GET_OPT_H__

#include "common.h"

#define jc_no_argument          0
#define jc_required_argument    1
#define jc_optional_argument    2

struct jc_option {
    const char * name;
    int has_arg;
    int * flag;
    int val;
};

extern char jc_optarg[514];
int jc_getopt_long_only(int ___argc, char * const * ___argv, const char * __shortopts, const struct jc_option * __longopts, int * __longind);

#endif