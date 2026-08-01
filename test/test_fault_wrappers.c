#include <errno.h>
#include <p101_env/env.h>
#include <p101_error/error.h>
#include <p101_terminal/terminal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int failures;

#define EXPECT(condition)                                                                                                                                                                                                                                          \
    do                                                                                                                                                                                                                                                             \
    {                                                                                                                                                                                                                                                              \
        if(!(condition))                                                                                                                                                                                                                                           \
        {                                                                                                                                                                                                                                                          \
            fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #condition);                                                                                                                                                                                   \
            failures++;                                                                                                                                                                                                                                            \
        }                                                                                                                                                                                                                                                          \
    } while(0)

struct fault_state
{
    int checks;
    int errnum;
};

static int fail_next_call(const struct p101_env *env, const char *call_name, void *user_data)
{
    struct fault_state *state;

    (void)env;
    (void)call_name;
    state = user_data;
    state->checks++;
    return state->errnum;
}

/* P101_TEST_CASE(p101_cfsetispeed) */
static void test_p101_cfsetispeed(struct p101_env *env, struct p101_error *err)
{
#ifdef __linux__
    static const int errors[] = {EIO};
#elif defined(__APPLE__)
    static const int errors[] = {EIO};
#elif defined(__FreeBSD__)
    static const int errors[] = {EIO};
#else
    static const int errors[] = {EINVAL};
#endif

    for(size_t index = 0U; index < sizeof(errors) / sizeof(errors[0]); index++)
    {
        struct fault_state state = {0, errors[index]};

        p101_env_set_fault_injector(env, fail_next_call, &state);
        int result = p101_cfsetispeed(env, err, NULL, 0);
        (void)result;
        EXPECT(state.checks == 1);
        EXPECT(p101_error_is_errno(err, state.errnum));
        p101_error_reset(err);
    }
    p101_env_set_fault_injector(env, NULL, NULL);
}

/* P101_TEST_CASE(p101_cfsetospeed) */
static void test_p101_cfsetospeed(struct p101_env *env, struct p101_error *err)
{
#ifdef __linux__
    static const int errors[] = {EIO};
#elif defined(__APPLE__)
    static const int errors[] = {EIO};
#elif defined(__FreeBSD__)
    static const int errors[] = {EIO};
#else
    static const int errors[] = {EINVAL};
#endif

    for(size_t index = 0U; index < sizeof(errors) / sizeof(errors[0]); index++)
    {
        struct fault_state state = {0, errors[index]};

        p101_env_set_fault_injector(env, fail_next_call, &state);
        int result = p101_cfsetospeed(env, err, NULL, 0);
        (void)result;
        EXPECT(state.checks == 1);
        EXPECT(p101_error_is_errno(err, state.errnum));
        p101_error_reset(err);
    }
    p101_env_set_fault_injector(env, NULL, NULL);
}

/* P101_TEST_CASE(p101_cfsetspeed) */
static void test_p101_cfsetspeed(struct p101_env *env, struct p101_error *err)
{
#ifdef __linux__
    static const int errors[] = {EIO};
#elif defined(__APPLE__)
    static const int errors[] = {EIO};
#elif defined(__FreeBSD__)
    static const int errors[] = {EIO};
#else
    static const int errors[] = {EIO};
#endif

    for(size_t index = 0U; index < sizeof(errors) / sizeof(errors[0]); index++)
    {
        struct fault_state state = {0, errors[index]};

        p101_env_set_fault_injector(env, fail_next_call, &state);
        int result = p101_cfsetspeed(env, err, NULL, 0);
        (void)result;
        EXPECT(state.checks == 1);
        EXPECT(p101_error_is_errno(err, state.errnum));
        p101_error_reset(err);
    }
    p101_env_set_fault_injector(env, NULL, NULL);
}

/* P101_TEST_CASE(p101_grantpt) */
static void test_p101_grantpt(struct p101_env *env, struct p101_error *err)
{
#ifdef __linux__
    static const int errors[] = {EACCES, EBADF, EINVAL};
#elif defined(__APPLE__)
    static const int errors[] = {EACCES, EBADF, EFAULT, EINVAL, EIO, EOVERFLOW, ERANGE};
#elif defined(__FreeBSD__)
    static const int errors[] = {EACCES, EBADF, EINVAL};
#else
    static const int errors[] = {EACCES, EBADF, EINVAL};
#endif

    for(size_t index = 0U; index < sizeof(errors) / sizeof(errors[0]); index++)
    {
        struct fault_state state = {0, errors[index]};

        p101_env_set_fault_injector(env, fail_next_call, &state);
        int result = p101_grantpt(env, err, 0);
        (void)result;
        EXPECT(state.checks == 1);
        EXPECT(p101_error_is_errno(err, state.errnum));
        p101_error_reset(err);
    }
    p101_env_set_fault_injector(env, NULL, NULL);
}

/* P101_TEST_CASE(p101_isatty) */
static void test_p101_isatty(struct p101_env *env, struct p101_error *err)
{
#ifdef __linux__
    static const int errors[] = {EBADF, ENOTTY};
#elif defined(__APPLE__)
    static const int errors[] = {EIO};
#elif defined(__FreeBSD__)
    static const int errors[] = {EIO};
#else
    static const int errors[] = {EBADF, ENOTTY};
#endif

    for(size_t index = 0U; index < sizeof(errors) / sizeof(errors[0]); index++)
    {
        struct fault_state state = {0, errors[index]};

        p101_env_set_fault_injector(env, fail_next_call, &state);
        int result = p101_isatty(env, err, 0);
        (void)result;
        EXPECT(state.checks == 1);
        EXPECT(p101_error_is_errno(err, state.errnum));
        p101_error_reset(err);
    }
    p101_env_set_fault_injector(env, NULL, NULL);
}

/* P101_TEST_CASE(p101_posix_openpt) */
static void test_p101_posix_openpt(struct p101_env *env, struct p101_error *err)
{
#ifdef __linux__
    static const int errors[] = {EIO};
#elif defined(__APPLE__)
    static const int errors[] = {EACCES, EAGAIN, EBADF, EDEADLK, EDQUOT, EEXIST, EFAULT, EILSEQ, EINTR, EINVAL, EIO, EISDIR, ELOOP, EMFILE, ENAMETOOLONG, ENFILE, ENOENT, ENOSPC, ENOTDIR, ENXIO, EOPNOTSUPP, EOVERFLOW, ERANGE, EROFS, ETXTBSY, EWOULDBLOCK};
#elif defined(__FreeBSD__)
    static const int errors[] = {EAGAIN, EINVAL, ENFILE};
#else
    static const int errors[] = {EAGAIN, EINVAL, EMFILE, ENFILE};
#endif

    for(size_t index = 0U; index < sizeof(errors) / sizeof(errors[0]); index++)
    {
        struct fault_state state = {0, errors[index]};

        p101_env_set_fault_injector(env, fail_next_call, &state);
        int result = p101_posix_openpt(env, err, 0);
        (void)result;
        EXPECT(state.checks == 1);
        EXPECT(p101_error_is_errno(err, state.errnum));
        p101_error_reset(err);
    }
    p101_env_set_fault_injector(env, NULL, NULL);
}

/* P101_TEST_CASE(p101_ptsname) */
static void test_p101_ptsname(struct p101_env *env, struct p101_error *err)
{
#ifdef __linux__
    static const int errors[] = {EIO};
#elif defined(__APPLE__)
    static const int errors[] = {EBADF, EFAULT, EINVAL, EIO, EOVERFLOW, ERANGE};
#elif defined(__FreeBSD__)
    static const int errors[] = {EBADF, EINVAL};
#else
    static const int errors[] = {EBADF, EINVAL, ENOTTY};
#endif

    for(size_t index = 0U; index < sizeof(errors) / sizeof(errors[0]); index++)
    {
        struct fault_state state = {0, errors[index]};

        p101_env_set_fault_injector(env, fail_next_call, &state);
        char *result = p101_ptsname(env, err, 0);
        (void)result;
        EXPECT(state.checks == 1);
        EXPECT(p101_error_is_errno(err, state.errnum));
        p101_error_reset(err);
    }
    p101_env_set_fault_injector(env, NULL, NULL);
}

/* P101_TEST_CASE(p101_tcdrain) */
static void test_p101_tcdrain(struct p101_env *env, struct p101_error *err)
{
#ifdef __linux__
    static const int errors[] = {EIO};
#elif defined(__APPLE__)
    static const int errors[] = {EBADF, EINTR, EINVAL, ENOTTY};
#elif defined(__FreeBSD__)
    static const int errors[] = {EBADF, EINTR, EINVAL, ENOTTY, EWOULDBLOCK};
#else
    static const int errors[] = {EBADF, EINTR, EIO, ENOTTY};
#endif

    for(size_t index = 0U; index < sizeof(errors) / sizeof(errors[0]); index++)
    {
        struct fault_state state = {0, errors[index]};

        p101_env_set_fault_injector(env, fail_next_call, &state);
        int result = p101_tcdrain(env, err, 0);
        (void)result;
        EXPECT(state.checks == 1);
        EXPECT(p101_error_is_errno(err, state.errnum));
        p101_error_reset(err);
    }
    p101_env_set_fault_injector(env, NULL, NULL);
}

/* P101_TEST_CASE(p101_tcflow) */
static void test_p101_tcflow(struct p101_env *env, struct p101_error *err)
{
#ifdef __linux__
    static const int errors[] = {EIO};
#elif defined(__APPLE__)
    static const int errors[] = {EIO};
#elif defined(__FreeBSD__)
    static const int errors[] = {EIO};
#else
    static const int errors[] = {EBADF, EINVAL, EIO, ENOTTY};
#endif

    for(size_t index = 0U; index < sizeof(errors) / sizeof(errors[0]); index++)
    {
        struct fault_state state = {0, errors[index]};

        p101_env_set_fault_injector(env, fail_next_call, &state);
        int result = p101_tcflow(env, err, 0, 0);
        (void)result;
        EXPECT(state.checks == 1);
        EXPECT(p101_error_is_errno(err, state.errnum));
        p101_error_reset(err);
    }
    p101_env_set_fault_injector(env, NULL, NULL);
}

/* P101_TEST_CASE(p101_tcflush) */
static void test_p101_tcflush(struct p101_env *env, struct p101_error *err)
{
#ifdef __linux__
    static const int errors[] = {EIO};
#elif defined(__APPLE__)
    static const int errors[] = {EIO};
#elif defined(__FreeBSD__)
    static const int errors[] = {EIO};
#else
    static const int errors[] = {EBADF, EINVAL, EIO, ENOTTY};
#endif

    for(size_t index = 0U; index < sizeof(errors) / sizeof(errors[0]); index++)
    {
        struct fault_state state = {0, errors[index]};

        p101_env_set_fault_injector(env, fail_next_call, &state);
        int result = p101_tcflush(env, err, 0, 0);
        (void)result;
        EXPECT(state.checks == 1);
        EXPECT(p101_error_is_errno(err, state.errnum));
        p101_error_reset(err);
    }
    p101_env_set_fault_injector(env, NULL, NULL);
}

/* P101_TEST_CASE(p101_tcgetattr) */
static void test_p101_tcgetattr(struct p101_env *env, struct p101_error *err)
{
#ifdef __linux__
    static const int errors[] = {EIO};
#elif defined(__APPLE__)
    static const int errors[] = {EIO};
#elif defined(__FreeBSD__)
    static const int errors[] = {EIO};
#else
    static const int errors[] = {EBADF, ENOTTY};
#endif

    for(size_t index = 0U; index < sizeof(errors) / sizeof(errors[0]); index++)
    {
        struct fault_state state = {0, errors[index]};

        p101_env_set_fault_injector(env, fail_next_call, &state);
        int result = p101_tcgetattr(env, err, 0, NULL);
        (void)result;
        EXPECT(state.checks == 1);
        EXPECT(p101_error_is_errno(err, state.errnum));
        p101_error_reset(err);
    }
    p101_env_set_fault_injector(env, NULL, NULL);
}

/* P101_TEST_CASE(p101_tcgetpgrp) */
static void test_p101_tcgetpgrp(struct p101_env *env, struct p101_error *err)
{
#ifdef __linux__
    static const int errors[] = {EIO};
#elif defined(__APPLE__)
    static const int errors[] = {EBADF, ENOTTY};
#elif defined(__FreeBSD__)
    static const int errors[] = {EBADF, ENOTTY};
#else
    static const int errors[] = {EBADF, ENOTTY};
#endif

    for(size_t index = 0U; index < sizeof(errors) / sizeof(errors[0]); index++)
    {
        struct fault_state state = {0, errors[index]};

        p101_env_set_fault_injector(env, fail_next_call, &state);
        pid_t result = p101_tcgetpgrp(env, err, 0);
        (void)result;
        EXPECT(state.checks == 1);
        EXPECT(p101_error_is_errno(err, state.errnum));
        p101_error_reset(err);
    }
    p101_env_set_fault_injector(env, NULL, NULL);
}

/* P101_TEST_CASE(p101_tcgetsid) */
static void test_p101_tcgetsid(struct p101_env *env, struct p101_error *err)
{
#ifdef __linux__
    static const int errors[] = {EBADF, ENOTTY};
#elif defined(__APPLE__)
    static const int errors[] = {EACCES, EBADF, ENOTTY};
#elif defined(__FreeBSD__)
    static const int errors[] = {EBADF, ENOTTY};
#else
    static const int errors[] = {EBADF, ENOTTY};
#endif

    for(size_t index = 0U; index < sizeof(errors) / sizeof(errors[0]); index++)
    {
        struct fault_state state = {0, errors[index]};

        p101_env_set_fault_injector(env, fail_next_call, &state);
        pid_t result = p101_tcgetsid(env, err, 0);
        (void)result;
        EXPECT(state.checks == 1);
        EXPECT(p101_error_is_errno(err, state.errnum));
        p101_error_reset(err);
    }
    p101_env_set_fault_injector(env, NULL, NULL);
}

/* P101_TEST_CASE(p101_tcsendbreak) */
static void test_p101_tcsendbreak(struct p101_env *env, struct p101_error *err)
{
#ifdef __linux__
    static const int errors[] = {EIO};
#elif defined(__APPLE__)
    static const int errors[] = {EIO};
#elif defined(__FreeBSD__)
    static const int errors[] = {EIO};
#else
    static const int errors[] = {EBADF, EIO, ENOTTY};
#endif

    for(size_t index = 0U; index < sizeof(errors) / sizeof(errors[0]); index++)
    {
        struct fault_state state = {0, errors[index]};

        p101_env_set_fault_injector(env, fail_next_call, &state);
        int result = p101_tcsendbreak(env, err, 0, 0);
        (void)result;
        EXPECT(state.checks == 1);
        EXPECT(p101_error_is_errno(err, state.errnum));
        p101_error_reset(err);
    }
    p101_env_set_fault_injector(env, NULL, NULL);
}

/* P101_TEST_CASE(p101_tcsetattr) */
static void test_p101_tcsetattr(struct p101_env *env, struct p101_error *err)
{
#ifdef __linux__
    static const int errors[] = {EIO};
#elif defined(__APPLE__)
    static const int errors[] = {EIO};
#elif defined(__FreeBSD__)
    static const int errors[] = {EIO};
#else
    static const int errors[] = {EBADF, EINTR, EINVAL, EIO, ENOTTY};
#endif

    for(size_t index = 0U; index < sizeof(errors) / sizeof(errors[0]); index++)
    {
        struct fault_state state = {0, errors[index]};

        p101_env_set_fault_injector(env, fail_next_call, &state);
        int result = p101_tcsetattr(env, err, 0, 0, NULL);
        (void)result;
        EXPECT(state.checks == 1);
        EXPECT(p101_error_is_errno(err, state.errnum));
        p101_error_reset(err);
    }
    p101_env_set_fault_injector(env, NULL, NULL);
}

/* P101_TEST_CASE(p101_tcsetpgrp) */
static void test_p101_tcsetpgrp(struct p101_env *env, struct p101_error *err)
{
#ifdef __linux__
    static const int errors[] = {EBADF, EINVAL, ENOTTY, EPERM};
#elif defined(__APPLE__)
    static const int errors[] = {EBADF, EINVAL, ENOTTY, EPERM};
#elif defined(__FreeBSD__)
    static const int errors[] = {EBADF, EINVAL, ENOTTY, EPERM};
#else
    static const int errors[] = {EBADF, EINTR, EINVAL, EIO, ENOTTY, EPERM};
#endif

    for(size_t index = 0U; index < sizeof(errors) / sizeof(errors[0]); index++)
    {
        struct fault_state state = {0, errors[index]};

        p101_env_set_fault_injector(env, fail_next_call, &state);
        int result = p101_tcsetpgrp(env, err, 0, 0);
        (void)result;
        EXPECT(state.checks == 1);
        EXPECT(p101_error_is_errno(err, state.errnum));
        p101_error_reset(err);
    }
    p101_env_set_fault_injector(env, NULL, NULL);
}

/* P101_TEST_CASE(p101_ttyname_r) */
static void test_p101_ttyname_r(struct p101_env *env, struct p101_error *err)
{
#ifdef __linux__
    static const int errors[] = {EBADF, ENODEV, ENOTTY, ERANGE};
#elif defined(__APPLE__)
    static const int errors[] = {EBADF, ENOTTY, ERANGE};
#elif defined(__FreeBSD__)
    static const int errors[] = {EBADF, ENOTTY, ERANGE};
#else
    static const int errors[] = {EBADF, ENOTTY, ERANGE};
#endif

    for(size_t index = 0U; index < sizeof(errors) / sizeof(errors[0]); index++)
    {
        struct fault_state state = {0, errors[index]};

        p101_env_set_fault_injector(env, fail_next_call, &state);
        int result = p101_ttyname_r(env, err, 0, NULL, 0);
        (void)result;
        EXPECT(state.checks == 1);
        EXPECT(p101_error_is_errno(err, state.errnum));
        p101_error_reset(err);
    }
    p101_env_set_fault_injector(env, NULL, NULL);
}

/* P101_TEST_CASE(p101_unlockpt) */
static void test_p101_unlockpt(struct p101_env *env, struct p101_error *err)
{
#ifdef __linux__
    static const int errors[] = {EBADF, EINVAL};
#elif defined(__APPLE__)
    static const int errors[] = {EBADF, EFAULT, EINVAL, EIO, EOVERFLOW, ERANGE};
#elif defined(__FreeBSD__)
    static const int errors[] = {EBADF, EINVAL};
#else
    static const int errors[] = {EBADF, EINVAL};
#endif

    for(size_t index = 0U; index < sizeof(errors) / sizeof(errors[0]); index++)
    {
        struct fault_state state = {0, errors[index]};

        p101_env_set_fault_injector(env, fail_next_call, &state);
        int result = p101_unlockpt(env, err, 0);
        (void)result;
        EXPECT(state.checks == 1);
        EXPECT(p101_error_is_errno(err, state.errnum));
        p101_error_reset(err);
    }
    p101_env_set_fault_injector(env, NULL, NULL);
}

int main(void)
{
    struct p101_error *err;
    struct p101_env   *env;

    err = p101_error_create(false);
    if(err == NULL)
    {
        return EXIT_FAILURE;
    }
    env = p101_env_create(err, NULL);
    if(env == NULL)
    {
        p101_error_destroy(err);
        return EXIT_FAILURE;
    }
    test_p101_cfsetispeed(env, err);
    test_p101_cfsetospeed(env, err);
    test_p101_cfsetspeed(env, err);
    test_p101_grantpt(env, err);
    test_p101_isatty(env, err);
    test_p101_posix_openpt(env, err);
    test_p101_ptsname(env, err);
    test_p101_tcdrain(env, err);
    test_p101_tcflow(env, err);
    test_p101_tcflush(env, err);
    test_p101_tcgetattr(env, err);
    test_p101_tcgetpgrp(env, err);
    test_p101_tcgetsid(env, err);
    test_p101_tcsendbreak(env, err);
    test_p101_tcsetattr(env, err);
    test_p101_tcsetpgrp(env, err);
    test_p101_ttyname_r(env, err);
    test_p101_unlockpt(env, err);
    p101_env_destroy(env);
    p101_error_destroy(err);
    return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
