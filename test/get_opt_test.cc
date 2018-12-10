#include "gtest/gtest.h"
#include "get_opt.h"

TEST(GetOptTest, use1)
{
    char * proc1[1] = { "main.out" };
    char * const * argv = proc1;
    int argc = 1;

    int a = 0, b = 0, d = 0;
    struct jc_option longopts[] = {
        { "a",  jc_required_argument, &a, 1 },
        { "b",	jc_required_argument, &b, 1 },
        { "c",	jc_required_argument, nullptr, 'c' },
        { "d",	jc_no_argument, &d, 1 },
        { "e",	jc_required_argument, nullptr, -1 },
        { "f",	jc_no_argument, nullptr, 'f' },
        { nullptr, 0, nullptr, 0 }
    };

    char c;
    while ((c = static_cast<char>(jc_getopt_long_only(argc, argv, ":f:", longopts, nullptr))) != -1) {
        switch (c)
        {
        case 'c':
            {
                size_t len = strlen(jc_optarg);
                char * config_file = static_cast<char *>(jc_malloc(len + 1));
                strncpy(config_file, jc_optarg, len);
                config_file[len] = 0;
                jc_free(config_file);
            }
            break;

        case 0:
            break;

        default:
            printf("Usage: %s [--a <level>] [--b <filename>] [--c <filename>] [--d] [--e <filename>] [--f <filename>]\n", argv[0]);
            return;
        }
    }
}

TEST(GetOptTest, use2)
{
    char * proc2[2] = { "main.out", "error_arg" };
    char * const * argv = proc2;
    int argc = 2;

    int a = 0, b = 0, d = 0;
    struct jc_option longopts[] = {
        { "a",  jc_required_argument, &a, 1 },
        { "b",	jc_required_argument, &b, 1 },
        { "c",	jc_required_argument, nullptr, 'c' },
        { "d",	jc_no_argument, &d, 1 },
        { "e",	jc_required_argument, nullptr, -1 },
        { "f",	jc_no_argument, nullptr, 'f' },
        { nullptr, 0, nullptr, 0 }
    };

    char c;
    while ((c = static_cast<char>(jc_getopt_long_only(argc, argv, ":f:", longopts, nullptr))) != -1) {
        switch (c)
        {
        case 'c':
        {
            size_t len = strlen(jc_optarg);
            char * config_file = static_cast<char *>(jc_malloc(len + 1));
            strncpy(config_file, jc_optarg, len);
            config_file[len] = 0;
            jc_free(config_file);
        }
        break;

        case 0:
            break;

        default:
            printf("Usage: %s [--a <level>] [--b <filename>] [--c <filename>] [--d] [--e <filename>] [--f <filename>]\n", argv[0]);
            return;
        }
    }
}

TEST(GetOptTest, use3)
{
    char * proc3[2] = { "main.out", "--not_support_arg" };
    char * const * argv = proc3;
    int argc = 2;

    int a = 0, b = 0, d = 0;
    struct jc_option longopts[] = {
        { "a",  jc_required_argument, &a, 1 },
        { "b",	jc_required_argument, &b, 1 },
        { "c",	jc_required_argument, nullptr, 'c' },
        { "d",	jc_no_argument, &d, 1 },
        { "e",	jc_required_argument, nullptr, -1 },
        { "f",	jc_no_argument, nullptr, 'f' },
        { nullptr, 0, nullptr, 0 }
    };

    char c;
    while ((c = static_cast<char>(jc_getopt_long_only(argc, argv, ":f:", longopts, nullptr))) != -1) {
        switch (c)
        {
        case 'c':
        {
            size_t len = strlen(jc_optarg);
            char * config_file = static_cast<char *>(jc_malloc(len + 1));
            strncpy(config_file, jc_optarg, len);
            config_file[len] = 0;
            jc_free(config_file);
        }
        break;

        case 0:
            break;

        default:
            printf("Usage: %s [--a <level>] [--b <filename>] [--c <filename>] [--d] [--e <filename>] [--f <filename>]\n", argv[0]);
            return;
        }
    }
}

TEST(GetOptTest, use4)
{
    char * proc4[5] = { "main.out", "--a", "1", "--b", "--c" };
    char * const * argv = proc4;
    int argc = 5;

    int a = 0, b = 0, d = 0;
    struct jc_option longopts[] = {
        { "a",  jc_required_argument, &a, 1 },
        { "b",	jc_required_argument, &b, 1 },
        { "c",	jc_required_argument, nullptr, 'c' },
        { "d",	jc_no_argument, &d, 1 },
        { "e",	jc_required_argument, nullptr, -1 },
        { "f",	jc_no_argument, nullptr, 'f' },
        { nullptr, 0, nullptr, 0 }
    };

    char c;
    while ((c = static_cast<char>(jc_getopt_long_only(argc, argv, ":f:", longopts, nullptr))) != -1) {
        switch (c)
        {
        case 'c':
        {
            size_t len = strlen(jc_optarg);
            char * config_file = static_cast<char *>(jc_malloc(len + 1));
            strncpy(config_file, jc_optarg, len);
            config_file[len] = 0;
            jc_free(config_file);
        }
        break;

        case 0:
            break;

        default:
            printf("Usage: %s [--a <level>] [--b <filename>] [--c <filename>] [--d] [--e <filename>] [--f <filename>]\n", argv[0]);
            return;
        }
    }
}

TEST(GetOptTest, use5)
{
    char * proc5[9] = { "main.out", "--a", "1", "--b", "--c", "abc", "--d", "--e", "efg" };
    char * const * argv = proc5;
    int argc = 9;

    int a = 0, b = 0, d = 0;
    struct jc_option longopts[] = {
        { "a",  jc_required_argument, &a, 1 },
        { "b",	jc_required_argument, &b, 1 },
        { "c",	jc_required_argument, nullptr, 'c' },
        { "d",	jc_no_argument, &d, 1 },
        { "e",	jc_required_argument, nullptr, -1 },
        { "f",	jc_no_argument, nullptr, 'f' },
        { nullptr, 0, nullptr, 0 }
    };

    char c;
    while ((c = static_cast<char>(jc_getopt_long_only(argc, argv, ":f:", longopts, nullptr))) != -1) {
        switch (c)
        {
        case 'c':
        {
            size_t len = strlen(jc_optarg);
            char * config_file = static_cast<char *>(jc_malloc(len + 1));
            strncpy(config_file, jc_optarg, len);
            config_file[len] = 0;
            jc_free(config_file);
        }
        break;

        case 0:
            break;

        default:
            printf("Usage: %s [--a <level>] [--b <filename>] [--c <filename>] [--d] [--e <filename>] [--f <filename>]\n", argv[0]);
            return;
        }
    }
}

TEST(GetOptTest, use6)
{
    char * proc6[10] = { "main.out", "--a", "1", "--b", "--c", "abc", "--d", "--e", "efg", "--f" };
    char * const * argv = proc6;
    int argc = 10;

    int a = 0, b = 0, d = 0;
    struct jc_option longopts[] = {
        { "a",  jc_required_argument, &a, 1 },
        { "b",	jc_required_argument, &b, 1 },
        { "c",	jc_required_argument, nullptr, 'c' },
        { "d",	jc_no_argument, &d, 1 },
        { "e",	jc_required_argument, nullptr, -1 },
        { "f",	jc_no_argument, nullptr, 'f' },
        { nullptr, 0, nullptr, 0 }
    };

    char c;
    while ((c = static_cast<char>(jc_getopt_long_only(argc, argv, ":f:", longopts, nullptr))) != -1) {
        switch (c)
        {
        case 'c':
        {
            size_t len = strlen(jc_optarg);
            char * config_file = static_cast<char *>(jc_malloc(len + 1));
            strncpy(config_file, jc_optarg, len);
            config_file[len] = 0;
            jc_free(config_file);
        }
        break;

        case 0:
            break;

        default:
            printf("Usage: %s [--a <level>] [--b <filename>] [--c <filename>] [--d] [--e <filename>] [--f <filename>]\n", argv[0]);
            return;
        }
    }
}