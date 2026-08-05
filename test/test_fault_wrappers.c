#include <arpa/inet.h>
#include <dirent.h>
#include <errno.h>
#include <fmtmsg.h>
#include <fnmatch.h>
#include <ftw.h>
#include <limits.h>
#include <math.h>
#include <p101_env/env.h>
#include <p101_error/error.h>
#include <p101_terminal/terminal.h>
#include <pthread.h>
#include <search.h>
#include <signal.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <utmpx.h>

static int    failures;
static size_t fault_resource_events;
static FILE  *outcome_stream;

#define P101_TEST_ERRNO_SENTINEL 0x5A5A

#ifdef __linux__
    #define P101_TEST_PLATFORM "linux"
#elif defined(__APPLE__)
    #define P101_TEST_PLATFORM "macos"
#elif defined(__FreeBSD__)
    #define P101_TEST_PLATFORM "freebsd"
#else
    #define P101_TEST_PLATFORM "posix"
#endif

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
    int code;
};

static void write_outcome(const char *wrapper, const char *domain, const char *symbol, int code, int passed)
{
    int written;

    if(outcome_stream == NULL)
    {
        return;
    }
    written = fprintf(outcome_stream, "P101WRAPPER\t1\tFAULT\t%s\tlib_terminal\t%s\t%s\t%s\t%d\t%s\n", P101_TEST_PLATFORM, wrapper, domain, symbol, code, passed ? "PASS" : "FAIL");
    if(written < 0 || fflush(outcome_stream) != 0)
    {
        fprintf(stderr, "FAIL: cannot write wrapper outcome receipt\n");
        failures++;
    }
}

static int fail_next_call(const struct p101_env *env, const char *call_name, void *user_data)
{
    struct fault_state *state;

    (void)env;
    (void)call_name;
    state = user_data;
    state->checks++;
    return state->code;
}

static void count_fd_event(const struct p101_env *env, p101_env_fd_event event, int fd, const char *file_name, const char *function_name, int line_number, void *user_data)
{
    (void)env;
    (void)event;
    (void)fd;
    (void)file_name;
    (void)function_name;
    (void)line_number;
    (void)user_data;
    fault_resource_events++;
}

static void count_alloc_event(const struct p101_env *env, p101_env_alloc_event event, const void *ptr, const void *new_ptr, size_t size, const char *file_name, const char *function_name, int line_number, void *user_data)
{
    (void)env;
    (void)event;
    (void)ptr;
    (void)new_ptr;
    (void)size;
    (void)file_name;
    (void)function_name;
    (void)line_number;
    (void)user_data;
    fault_resource_events++;
}

static void count_resource_event(const struct p101_env *env, p101_env_resource_kind event, const char *resource_class, const char *resource_id, const char *related_id, size_t size, const char *metadata, const char *file_name, const char *function_name,
                                 int line_number, void *user_data)
{
    (void)env;
    (void)event;
    (void)resource_class;
    (void)resource_id;
    (void)related_id;
    (void)size;
    (void)metadata;
    (void)file_name;
    (void)function_name;
    (void)line_number;
    (void)user_data;
    fault_resource_events++;
}

/* P101_TEST_CASE(p101_cfsetispeed) */
static void test_p101_cfsetispeed(struct p101_env *env, struct p101_error *err)
{
#ifdef __linux__
    static const int         errors[]      = {EINVAL};
    static const char *const error_names[] = {"EINVAL"};
#elif defined(__APPLE__)
    static const int         errors[]      = {EINVAL};
    static const char *const error_names[] = {"EINVAL"};
#elif defined(__FreeBSD__)
    static const int         errors[]      = {EINVAL};
    static const char *const error_names[] = {"EINVAL"};
#else
    static const int         errors[]      = {EINVAL};
    static const char *const error_names[] = {"EINVAL"};
#endif

    for(size_t index = 0U; index < sizeof(errors) / sizeof(errors[0]); index++)
    {
        struct fault_state state = {0, errors[index]};
        int                failures_before;

        failures_before = failures;
        EXPECT(p101_error_has_no_error(err));
        fault_resource_events = 0U;
        errno                 = P101_TEST_ERRNO_SENTINEL;
        p101_env_set_fault_injector(env, fail_next_call, &state);
        int result = p101_cfsetispeed(env, err, NULL, 0);
        (void)result;
        EXPECT(state.checks == 1);
        EXPECT(p101_error_is_errno(err, state.code));
        EXPECT(errno == P101_TEST_ERRNO_SENTINEL);
        EXPECT(result == (-1));
        EXPECT(fault_resource_events == 0U);
        write_outcome("p101_cfsetispeed", "errno", error_names[index], state.code, failures == failures_before);
        p101_error_reset(err);
    }
    p101_env_set_fault_injector(env, NULL, NULL);
    {
        int   native_status = 0;
        pid_t native_pid    = fork();

        EXPECT(native_pid >= 0);
        if(native_pid == 0)
        {
            struct p101_error *native_err;
            struct p101_env   *native_env;

            (void)alarm(2U);
            (void)unsetenv("P101_CALL_LOG");
            (void)unsetenv("P101_RESOURCE_LOG");
            native_err = p101_error_create(false);
            if(native_err == NULL)
            {
                _Exit(77);
            }
            native_env = p101_env_create(native_err, NULL);
            if(native_env == NULL)
            {
                p101_error_destroy(native_err);
                _Exit(77);
            }
            struct termios native_argument_2 = {0};
            int            native_result     = p101_cfsetispeed(native_env, native_err, &native_argument_2, 0);
            (void)native_result;
            p101_env_destroy(native_env);
            p101_error_destroy(native_err);
            _Exit(EXIT_SUCCESS);
        }
        if(native_pid > 0)
        {
            EXPECT(waitpid(native_pid, &native_status, 0) == native_pid);
            EXPECT(WIFEXITED(native_status));
            if(WIFEXITED(native_status))
            {
                EXPECT(WEXITSTATUS(native_status) == EXIT_SUCCESS);
            }
        }
        p101_error_reset(err);
    }
}

/* P101_TEST_CASE(p101_cfsetospeed) */
static void test_p101_cfsetospeed(struct p101_env *env, struct p101_error *err)
{
#ifdef __linux__
    static const int         errors[]      = {EINVAL};
    static const char *const error_names[] = {"EINVAL"};
#elif defined(__APPLE__)
    static const int         errors[]      = {EINVAL};
    static const char *const error_names[] = {"EINVAL"};
#elif defined(__FreeBSD__)
    static const int         errors[]      = {EINVAL};
    static const char *const error_names[] = {"EINVAL"};
#else
    static const int         errors[]      = {EINVAL};
    static const char *const error_names[] = {"EINVAL"};
#endif

    for(size_t index = 0U; index < sizeof(errors) / sizeof(errors[0]); index++)
    {
        struct fault_state state = {0, errors[index]};
        int                failures_before;

        failures_before = failures;
        EXPECT(p101_error_has_no_error(err));
        fault_resource_events = 0U;
        errno                 = P101_TEST_ERRNO_SENTINEL;
        p101_env_set_fault_injector(env, fail_next_call, &state);
        int result = p101_cfsetospeed(env, err, NULL, 0);
        (void)result;
        EXPECT(state.checks == 1);
        EXPECT(p101_error_is_errno(err, state.code));
        EXPECT(errno == P101_TEST_ERRNO_SENTINEL);
        EXPECT(result == (-1));
        EXPECT(fault_resource_events == 0U);
        write_outcome("p101_cfsetospeed", "errno", error_names[index], state.code, failures == failures_before);
        p101_error_reset(err);
    }
    p101_env_set_fault_injector(env, NULL, NULL);
    {
        int   native_status = 0;
        pid_t native_pid    = fork();

        EXPECT(native_pid >= 0);
        if(native_pid == 0)
        {
            struct p101_error *native_err;
            struct p101_env   *native_env;

            (void)alarm(2U);
            (void)unsetenv("P101_CALL_LOG");
            (void)unsetenv("P101_RESOURCE_LOG");
            native_err = p101_error_create(false);
            if(native_err == NULL)
            {
                _Exit(77);
            }
            native_env = p101_env_create(native_err, NULL);
            if(native_env == NULL)
            {
                p101_error_destroy(native_err);
                _Exit(77);
            }
            struct termios native_argument_2 = {0};
            int            native_result     = p101_cfsetospeed(native_env, native_err, &native_argument_2, 0);
            (void)native_result;
            p101_env_destroy(native_env);
            p101_error_destroy(native_err);
            _Exit(EXIT_SUCCESS);
        }
        if(native_pid > 0)
        {
            EXPECT(waitpid(native_pid, &native_status, 0) == native_pid);
            EXPECT(WIFEXITED(native_status));
            if(WIFEXITED(native_status))
            {
                EXPECT(WEXITSTATUS(native_status) == EXIT_SUCCESS);
            }
        }
        p101_error_reset(err);
    }
}

/* P101_TEST_CASE(p101_cfsetspeed) */
static void test_p101_cfsetspeed(struct p101_env *env, struct p101_error *err)
{
#ifdef __linux__
    static const int         errors[]      = {EIO};
    static const char *const error_names[] = {"EIO"};
#elif defined(__APPLE__)
    static const int         errors[]      = {EIO};
    static const char *const error_names[] = {"EIO"};
#elif defined(__FreeBSD__)
    static const int         errors[]      = {EIO};
    static const char *const error_names[] = {"EIO"};
#else
    static const int         errors[]      = {EIO};
    static const char *const error_names[] = {"EIO"};
#endif

    for(size_t index = 0U; index < sizeof(errors) / sizeof(errors[0]); index++)
    {
        struct fault_state state = {0, errors[index]};
        int                failures_before;

        failures_before = failures;
        EXPECT(p101_error_has_no_error(err));
        fault_resource_events = 0U;
        errno                 = P101_TEST_ERRNO_SENTINEL;
        p101_env_set_fault_injector(env, fail_next_call, &state);
        int result = p101_cfsetspeed(env, err, NULL, 0);
        (void)result;
        EXPECT(state.checks == 1);
        EXPECT(p101_error_is_errno(err, state.code));
        EXPECT(errno == P101_TEST_ERRNO_SENTINEL);
        EXPECT(result == (-1));
        EXPECT(fault_resource_events == 0U);
        write_outcome("p101_cfsetspeed", "errno", error_names[index], state.code, failures == failures_before);
        p101_error_reset(err);
    }
    p101_env_set_fault_injector(env, NULL, NULL);
    {
        int   native_status = 0;
        pid_t native_pid    = fork();

        EXPECT(native_pid >= 0);
        if(native_pid == 0)
        {
            struct p101_error *native_err;
            struct p101_env   *native_env;

            (void)alarm(2U);
            (void)unsetenv("P101_CALL_LOG");
            (void)unsetenv("P101_RESOURCE_LOG");
            native_err = p101_error_create(false);
            if(native_err == NULL)
            {
                _Exit(77);
            }
            native_env = p101_env_create(native_err, NULL);
            if(native_env == NULL)
            {
                p101_error_destroy(native_err);
                _Exit(77);
            }
            struct termios native_argument_2 = {0};
            int            native_result     = p101_cfsetspeed(native_env, native_err, &native_argument_2, 0);
            (void)native_result;
            p101_env_destroy(native_env);
            p101_error_destroy(native_err);
            _Exit(EXIT_SUCCESS);
        }
        if(native_pid > 0)
        {
            EXPECT(waitpid(native_pid, &native_status, 0) == native_pid);
            EXPECT(WIFEXITED(native_status));
            if(WIFEXITED(native_status))
            {
                EXPECT(WEXITSTATUS(native_status) == EXIT_SUCCESS);
            }
        }
        p101_error_reset(err);
    }
}

/* P101_TEST_CASE(p101_grantpt) */
static void test_p101_grantpt(struct p101_env *env, struct p101_error *err)
{
#ifdef __linux__
    static const int         errors[]      = {EACCES, EBADF, EINVAL};
    static const char *const error_names[] = {"EACCES", "EBADF", "EINVAL"};
#elif defined(__APPLE__)
    static const int         errors[]      = {EACCES, EBADF, EFAULT, EINVAL, EIO, EOVERFLOW, ERANGE};
    static const char *const error_names[] = {"EACCES", "EBADF", "EFAULT", "EINVAL", "EIO", "EOVERFLOW", "ERANGE"};
#elif defined(__FreeBSD__)
    static const int         errors[]      = {EACCES, EBADF, EINVAL};
    static const char *const error_names[] = {"EACCES", "EBADF", "EINVAL"};
#else
    static const int         errors[]      = {EACCES, EBADF, EINVAL};
    static const char *const error_names[] = {"EACCES", "EBADF", "EINVAL"};
#endif

    for(size_t index = 0U; index < sizeof(errors) / sizeof(errors[0]); index++)
    {
        struct fault_state state = {0, errors[index]};
        int                failures_before;

        failures_before = failures;
        EXPECT(p101_error_has_no_error(err));
        fault_resource_events = 0U;
        errno                 = P101_TEST_ERRNO_SENTINEL;
        p101_env_set_fault_injector(env, fail_next_call, &state);
        int result = p101_grantpt(env, err, 0);
        (void)result;
        EXPECT(state.checks == 1);
        EXPECT(p101_error_is_errno(err, state.code));
        EXPECT(errno == P101_TEST_ERRNO_SENTINEL);
        EXPECT(result == (-1));
        EXPECT(fault_resource_events == 0U);
        write_outcome("p101_grantpt", "errno", error_names[index], state.code, failures == failures_before);
        p101_error_reset(err);
    }
    p101_env_set_fault_injector(env, NULL, NULL);
    {
        int   native_status = 0;
        pid_t native_pid    = fork();

        EXPECT(native_pid >= 0);
        if(native_pid == 0)
        {
            struct p101_error *native_err;
            struct p101_env   *native_env;

            (void)alarm(2U);
            (void)unsetenv("P101_CALL_LOG");
            (void)unsetenv("P101_RESOURCE_LOG");
            native_err = p101_error_create(false);
            if(native_err == NULL)
            {
                _Exit(77);
            }
            native_env = p101_env_create(native_err, NULL);
            if(native_env == NULL)
            {
                p101_error_destroy(native_err);
                _Exit(77);
            }
            int native_result = p101_grantpt(native_env, native_err, 0);
            (void)native_result;
            p101_env_destroy(native_env);
            p101_error_destroy(native_err);
            _Exit(EXIT_SUCCESS);
        }
        if(native_pid > 0)
        {
            EXPECT(waitpid(native_pid, &native_status, 0) == native_pid);
            EXPECT(WIFEXITED(native_status));
            if(WIFEXITED(native_status))
            {
                EXPECT(WEXITSTATUS(native_status) == EXIT_SUCCESS);
            }
        }
        p101_error_reset(err);
    }
}

/* P101_TEST_CASE(p101_isatty) */
static void test_p101_isatty(struct p101_env *env, struct p101_error *err)
{
#ifdef __linux__
    static const int         errors[]      = {EBADF, ENOTTY};
    static const char *const error_names[] = {"EBADF", "ENOTTY"};
#elif defined(__APPLE__)
    static const int         errors[]      = {EBADF, ENOTTY};
    static const char *const error_names[] = {"EBADF", "ENOTTY"};
#elif defined(__FreeBSD__)
    static const int         errors[]      = {EBADF, ENOTTY};
    static const char *const error_names[] = {"EBADF", "ENOTTY"};
#else
    static const int         errors[]      = {EBADF, ENOTTY};
    static const char *const error_names[] = {"EBADF", "ENOTTY"};
#endif

    for(size_t index = 0U; index < sizeof(errors) / sizeof(errors[0]); index++)
    {
        struct fault_state state = {0, errors[index]};
        int                failures_before;

        failures_before = failures;
        EXPECT(p101_error_has_no_error(err));
        fault_resource_events = 0U;
        errno                 = P101_TEST_ERRNO_SENTINEL;
        p101_env_set_fault_injector(env, fail_next_call, &state);
        int result = p101_isatty(env, err, 0);
        (void)result;
        EXPECT(state.checks == 1);
        EXPECT(p101_error_is_errno(err, state.code));
        EXPECT(errno == P101_TEST_ERRNO_SENTINEL);
        EXPECT(result == (-1));
        EXPECT(fault_resource_events == 0U);
        write_outcome("p101_isatty", "errno", error_names[index], state.code, failures == failures_before);
        p101_error_reset(err);
    }
    p101_env_set_fault_injector(env, NULL, NULL);
    {
        int   native_status = 0;
        pid_t native_pid    = fork();

        EXPECT(native_pid >= 0);
        if(native_pid == 0)
        {
            struct p101_error *native_err;
            struct p101_env   *native_env;

            (void)alarm(2U);
            (void)unsetenv("P101_CALL_LOG");
            (void)unsetenv("P101_RESOURCE_LOG");
            native_err = p101_error_create(false);
            if(native_err == NULL)
            {
                _Exit(77);
            }
            native_env = p101_env_create(native_err, NULL);
            if(native_env == NULL)
            {
                p101_error_destroy(native_err);
                _Exit(77);
            }
            int native_result = p101_isatty(native_env, native_err, 0);
            (void)native_result;
            p101_env_destroy(native_env);
            p101_error_destroy(native_err);
            _Exit(EXIT_SUCCESS);
        }
        if(native_pid > 0)
        {
            EXPECT(waitpid(native_pid, &native_status, 0) == native_pid);
            EXPECT(WIFEXITED(native_status));
            if(WIFEXITED(native_status))
            {
                EXPECT(WEXITSTATUS(native_status) == EXIT_SUCCESS);
            }
        }
        p101_error_reset(err);
    }
}

/* P101_TEST_CASE(p101_posix_openpt) */
static void test_p101_posix_openpt(struct p101_env *env, struct p101_error *err)
{
#ifdef __linux__
    static const int         errors[]      = {EAGAIN, EINVAL, EMFILE, ENFILE};
    static const char *const error_names[] = {"EAGAIN", "EINVAL", "EMFILE", "ENFILE"};
#elif defined(__APPLE__)
    static const int errors[] = {EACCES, EAGAIN, EBADF, EDEADLK, EDQUOT, EEXIST, EFAULT, EILSEQ, EINTR, EINVAL, EIO, EISDIR, ELOOP, EMFILE, ENAMETOOLONG, ENFILE, ENOENT, ENOSPC, ENOTDIR, ENXIO, EOPNOTSUPP, EOVERFLOW, ERANGE, EROFS, ETXTBSY, EWOULDBLOCK};
    static const char *const error_names[] = {"EACCES", "EAGAIN",       "EBADF",  "EDEADLK", "EDQUOT", "EEXIST",  "EFAULT", "EILSEQ",     "EINTR",     "EINVAL", "EIO",   "EISDIR",  "ELOOP",
                                              "EMFILE", "ENAMETOOLONG", "ENFILE", "ENOENT",  "ENOSPC", "ENOTDIR", "ENXIO",  "EOPNOTSUPP", "EOVERFLOW", "ERANGE", "EROFS", "ETXTBSY", "EWOULDBLOCK"};
#elif defined(__FreeBSD__)
    static const int         errors[]      = {EAGAIN, EINVAL, ENFILE};
    static const char *const error_names[] = {"EAGAIN", "EINVAL", "ENFILE"};
#else
    static const int         errors[]      = {EAGAIN, EINVAL, EMFILE, ENFILE};
    static const char *const error_names[] = {"EAGAIN", "EINVAL", "EMFILE", "ENFILE"};
#endif

    for(size_t index = 0U; index < sizeof(errors) / sizeof(errors[0]); index++)
    {
        struct fault_state state = {0, errors[index]};
        int                failures_before;

        failures_before = failures;
        EXPECT(p101_error_has_no_error(err));
        fault_resource_events = 0U;
        errno                 = P101_TEST_ERRNO_SENTINEL;
        p101_env_set_fault_injector(env, fail_next_call, &state);
        int result = p101_posix_openpt(env, err, 0);
        (void)result;
        EXPECT(state.checks == 1);
        EXPECT(p101_error_is_errno(err, state.code));
        EXPECT(errno == P101_TEST_ERRNO_SENTINEL);
        EXPECT(result == (-1));
        EXPECT(fault_resource_events == 0U);
        write_outcome("p101_posix_openpt", "errno", error_names[index], state.code, failures == failures_before);
        p101_error_reset(err);
    }
    p101_env_set_fault_injector(env, NULL, NULL);
    {
        int   native_status = 0;
        pid_t native_pid    = fork();

        EXPECT(native_pid >= 0);
        if(native_pid == 0)
        {
            struct p101_error *native_err;
            struct p101_env   *native_env;

            (void)alarm(2U);
            (void)unsetenv("P101_CALL_LOG");
            (void)unsetenv("P101_RESOURCE_LOG");
            native_err = p101_error_create(false);
            if(native_err == NULL)
            {
                _Exit(77);
            }
            native_env = p101_env_create(native_err, NULL);
            if(native_env == NULL)
            {
                p101_error_destroy(native_err);
                _Exit(77);
            }
            int native_result = p101_posix_openpt(native_env, native_err, 0);
            (void)native_result;
            p101_env_destroy(native_env);
            p101_error_destroy(native_err);
            _Exit(EXIT_SUCCESS);
        }
        if(native_pid > 0)
        {
            EXPECT(waitpid(native_pid, &native_status, 0) == native_pid);
            EXPECT(WIFEXITED(native_status));
            if(WIFEXITED(native_status))
            {
                EXPECT(WEXITSTATUS(native_status) == EXIT_SUCCESS);
            }
        }
        p101_error_reset(err);
    }
}

/* P101_TEST_CASE(p101_ptsname) */
static void test_p101_ptsname(struct p101_env *env, struct p101_error *err)
{
#ifdef __linux__
    static const int         errors[]      = {EBADF, EINVAL, ENOTTY};
    static const char *const error_names[] = {"EBADF", "EINVAL", "ENOTTY"};
#elif defined(__APPLE__)
    static const int         errors[]      = {EBADF, EFAULT, EINVAL, EIO, EOVERFLOW, ERANGE};
    static const char *const error_names[] = {"EBADF", "EFAULT", "EINVAL", "EIO", "EOVERFLOW", "ERANGE"};
#elif defined(__FreeBSD__)
    static const int         errors[]      = {EBADF, EINVAL};
    static const char *const error_names[] = {"EBADF", "EINVAL"};
#else
    static const int         errors[]      = {EBADF, EINVAL, ENOTTY};
    static const char *const error_names[] = {"EBADF", "EINVAL", "ENOTTY"};
#endif

    for(size_t index = 0U; index < sizeof(errors) / sizeof(errors[0]); index++)
    {
        struct fault_state state = {0, errors[index]};
        int                failures_before;

        failures_before = failures;
        EXPECT(p101_error_has_no_error(err));
        fault_resource_events = 0U;
        errno                 = P101_TEST_ERRNO_SENTINEL;
        p101_env_set_fault_injector(env, fail_next_call, &state);
        char *result = p101_ptsname(env, err, 0);
        (void)result;
        EXPECT(state.checks == 1);
        EXPECT(p101_error_is_errno(err, state.code));
        EXPECT(errno == P101_TEST_ERRNO_SENTINEL);
        EXPECT(result == (NULL));
        EXPECT(fault_resource_events == 0U);
        write_outcome("p101_ptsname", "errno", error_names[index], state.code, failures == failures_before);
        p101_error_reset(err);
    }
    p101_env_set_fault_injector(env, NULL, NULL);
    {
        int   native_status = 0;
        pid_t native_pid    = fork();

        EXPECT(native_pid >= 0);
        if(native_pid == 0)
        {
            struct p101_error *native_err;
            struct p101_env   *native_env;

            (void)alarm(2U);
            (void)unsetenv("P101_CALL_LOG");
            (void)unsetenv("P101_RESOURCE_LOG");
            native_err = p101_error_create(false);
            if(native_err == NULL)
            {
                _Exit(77);
            }
            native_env = p101_env_create(native_err, NULL);
            if(native_env == NULL)
            {
                p101_error_destroy(native_err);
                _Exit(77);
            }
            char *native_result = p101_ptsname(native_env, native_err, 0);
            (void)native_result;
            p101_env_destroy(native_env);
            p101_error_destroy(native_err);
            _Exit(EXIT_SUCCESS);
        }
        if(native_pid > 0)
        {
            EXPECT(waitpid(native_pid, &native_status, 0) == native_pid);
            EXPECT(WIFEXITED(native_status));
            if(WIFEXITED(native_status))
            {
                EXPECT(WEXITSTATUS(native_status) == EXIT_SUCCESS);
            }
        }
        p101_error_reset(err);
    }
}

/* P101_TEST_CASE(p101_tcdrain) */
static void test_p101_tcdrain(struct p101_env *env, struct p101_error *err)
{
#ifdef __linux__
    static const int         errors[]      = {EBADF, EINTR, EIO, ENOTTY};
    static const char *const error_names[] = {"EBADF", "EINTR", "EIO", "ENOTTY"};
#elif defined(__APPLE__)
    static const int         errors[]      = {EBADF, EINTR, EINVAL, ENOTTY};
    static const char *const error_names[] = {"EBADF", "EINTR", "EINVAL", "ENOTTY"};
#elif defined(__FreeBSD__)
    static const int         errors[]      = {EBADF, EINTR, EINVAL, ENOTTY, EWOULDBLOCK};
    static const char *const error_names[] = {"EBADF", "EINTR", "EINVAL", "ENOTTY", "EWOULDBLOCK"};
#else
    static const int         errors[]      = {EBADF, EINTR, EIO, ENOTTY};
    static const char *const error_names[] = {"EBADF", "EINTR", "EIO", "ENOTTY"};
#endif

    for(size_t index = 0U; index < sizeof(errors) / sizeof(errors[0]); index++)
    {
        struct fault_state state = {0, errors[index]};
        int                failures_before;

        failures_before = failures;
        EXPECT(p101_error_has_no_error(err));
        fault_resource_events = 0U;
        errno                 = P101_TEST_ERRNO_SENTINEL;
        p101_env_set_fault_injector(env, fail_next_call, &state);
        int result = p101_tcdrain(env, err, 0);
        (void)result;
        EXPECT(state.checks == 1);
        EXPECT(p101_error_is_errno(err, state.code));
        EXPECT(errno == P101_TEST_ERRNO_SENTINEL);
        EXPECT(result == (-1));
        EXPECT(fault_resource_events == 0U);
        write_outcome("p101_tcdrain", "errno", error_names[index], state.code, failures == failures_before);
        p101_error_reset(err);
    }
    p101_env_set_fault_injector(env, NULL, NULL);
    {
        int   native_status = 0;
        pid_t native_pid    = fork();

        EXPECT(native_pid >= 0);
        if(native_pid == 0)
        {
            struct p101_error *native_err;
            struct p101_env   *native_env;

            (void)alarm(2U);
            (void)unsetenv("P101_CALL_LOG");
            (void)unsetenv("P101_RESOURCE_LOG");
            native_err = p101_error_create(false);
            if(native_err == NULL)
            {
                _Exit(77);
            }
            native_env = p101_env_create(native_err, NULL);
            if(native_env == NULL)
            {
                p101_error_destroy(native_err);
                _Exit(77);
            }
            int native_result = p101_tcdrain(native_env, native_err, 0);
            (void)native_result;
            p101_env_destroy(native_env);
            p101_error_destroy(native_err);
            _Exit(EXIT_SUCCESS);
        }
        if(native_pid > 0)
        {
            EXPECT(waitpid(native_pid, &native_status, 0) == native_pid);
            EXPECT(WIFEXITED(native_status));
            if(WIFEXITED(native_status))
            {
                EXPECT(WEXITSTATUS(native_status) == EXIT_SUCCESS);
            }
        }
        p101_error_reset(err);
    }
}

/* P101_TEST_CASE(p101_tcflow) */
static void test_p101_tcflow(struct p101_env *env, struct p101_error *err)
{
#ifdef __linux__
    static const int         errors[]      = {EBADF, EINVAL, EIO, ENOTTY};
    static const char *const error_names[] = {"EBADF", "EINVAL", "EIO", "ENOTTY"};
#elif defined(__APPLE__)
    static const int         errors[]      = {EBADF, EINVAL, EIO, ENOTTY};
    static const char *const error_names[] = {"EBADF", "EINVAL", "EIO", "ENOTTY"};
#elif defined(__FreeBSD__)
    static const int         errors[]      = {EBADF, EINVAL, EIO, ENOTTY};
    static const char *const error_names[] = {"EBADF", "EINVAL", "EIO", "ENOTTY"};
#else
    static const int         errors[]      = {EBADF, EINVAL, EIO, ENOTTY};
    static const char *const error_names[] = {"EBADF", "EINVAL", "EIO", "ENOTTY"};
#endif

    for(size_t index = 0U; index < sizeof(errors) / sizeof(errors[0]); index++)
    {
        struct fault_state state = {0, errors[index]};
        int                failures_before;

        failures_before = failures;
        EXPECT(p101_error_has_no_error(err));
        fault_resource_events = 0U;
        errno                 = P101_TEST_ERRNO_SENTINEL;
        p101_env_set_fault_injector(env, fail_next_call, &state);
        int result = p101_tcflow(env, err, 0, 0);
        (void)result;
        EXPECT(state.checks == 1);
        EXPECT(p101_error_is_errno(err, state.code));
        EXPECT(errno == P101_TEST_ERRNO_SENTINEL);
        EXPECT(result == (-1));
        EXPECT(fault_resource_events == 0U);
        write_outcome("p101_tcflow", "errno", error_names[index], state.code, failures == failures_before);
        p101_error_reset(err);
    }
    p101_env_set_fault_injector(env, NULL, NULL);
    {
        int   native_status = 0;
        pid_t native_pid    = fork();

        EXPECT(native_pid >= 0);
        if(native_pid == 0)
        {
            struct p101_error *native_err;
            struct p101_env   *native_env;

            (void)alarm(2U);
            (void)unsetenv("P101_CALL_LOG");
            (void)unsetenv("P101_RESOURCE_LOG");
            native_err = p101_error_create(false);
            if(native_err == NULL)
            {
                _Exit(77);
            }
            native_env = p101_env_create(native_err, NULL);
            if(native_env == NULL)
            {
                p101_error_destroy(native_err);
                _Exit(77);
            }
            int native_result = p101_tcflow(native_env, native_err, 0, 0);
            (void)native_result;
            p101_env_destroy(native_env);
            p101_error_destroy(native_err);
            _Exit(EXIT_SUCCESS);
        }
        if(native_pid > 0)
        {
            EXPECT(waitpid(native_pid, &native_status, 0) == native_pid);
            EXPECT(WIFEXITED(native_status));
            if(WIFEXITED(native_status))
            {
                EXPECT(WEXITSTATUS(native_status) == EXIT_SUCCESS);
            }
        }
        p101_error_reset(err);
    }
}

/* P101_TEST_CASE(p101_tcflush) */
static void test_p101_tcflush(struct p101_env *env, struct p101_error *err)
{
#ifdef __linux__
    static const int         errors[]      = {EBADF, EINVAL, EIO, ENOTTY};
    static const char *const error_names[] = {"EBADF", "EINVAL", "EIO", "ENOTTY"};
#elif defined(__APPLE__)
    static const int         errors[]      = {EBADF, EINVAL, EIO, ENOTTY};
    static const char *const error_names[] = {"EBADF", "EINVAL", "EIO", "ENOTTY"};
#elif defined(__FreeBSD__)
    static const int         errors[]      = {EBADF, EINVAL, EIO, ENOTTY};
    static const char *const error_names[] = {"EBADF", "EINVAL", "EIO", "ENOTTY"};
#else
    static const int         errors[]      = {EBADF, EINVAL, EIO, ENOTTY};
    static const char *const error_names[] = {"EBADF", "EINVAL", "EIO", "ENOTTY"};
#endif

    for(size_t index = 0U; index < sizeof(errors) / sizeof(errors[0]); index++)
    {
        struct fault_state state = {0, errors[index]};
        int                failures_before;

        failures_before = failures;
        EXPECT(p101_error_has_no_error(err));
        fault_resource_events = 0U;
        errno                 = P101_TEST_ERRNO_SENTINEL;
        p101_env_set_fault_injector(env, fail_next_call, &state);
        int result = p101_tcflush(env, err, 0, 0);
        (void)result;
        EXPECT(state.checks == 1);
        EXPECT(p101_error_is_errno(err, state.code));
        EXPECT(errno == P101_TEST_ERRNO_SENTINEL);
        EXPECT(result == (-1));
        EXPECT(fault_resource_events == 0U);
        write_outcome("p101_tcflush", "errno", error_names[index], state.code, failures == failures_before);
        p101_error_reset(err);
    }
    p101_env_set_fault_injector(env, NULL, NULL);
    {
        int   native_status = 0;
        pid_t native_pid    = fork();

        EXPECT(native_pid >= 0);
        if(native_pid == 0)
        {
            struct p101_error *native_err;
            struct p101_env   *native_env;

            (void)alarm(2U);
            (void)unsetenv("P101_CALL_LOG");
            (void)unsetenv("P101_RESOURCE_LOG");
            native_err = p101_error_create(false);
            if(native_err == NULL)
            {
                _Exit(77);
            }
            native_env = p101_env_create(native_err, NULL);
            if(native_env == NULL)
            {
                p101_error_destroy(native_err);
                _Exit(77);
            }
            int native_result = p101_tcflush(native_env, native_err, 0, 0);
            (void)native_result;
            p101_env_destroy(native_env);
            p101_error_destroy(native_err);
            _Exit(EXIT_SUCCESS);
        }
        if(native_pid > 0)
        {
            EXPECT(waitpid(native_pid, &native_status, 0) == native_pid);
            EXPECT(WIFEXITED(native_status));
            if(WIFEXITED(native_status))
            {
                EXPECT(WEXITSTATUS(native_status) == EXIT_SUCCESS);
            }
        }
        p101_error_reset(err);
    }
}

/* P101_TEST_CASE(p101_tcgetattr) */
static void test_p101_tcgetattr(struct p101_env *env, struct p101_error *err)
{
#ifdef __linux__
    static const int         errors[]      = {EBADF, ENOTTY};
    static const char *const error_names[] = {"EBADF", "ENOTTY"};
#elif defined(__APPLE__)
    static const int         errors[]      = {EBADF, ENOTTY};
    static const char *const error_names[] = {"EBADF", "ENOTTY"};
#elif defined(__FreeBSD__)
    static const int         errors[]      = {EBADF, ENOTTY};
    static const char *const error_names[] = {"EBADF", "ENOTTY"};
#else
    static const int         errors[]      = {EBADF, ENOTTY};
    static const char *const error_names[] = {"EBADF", "ENOTTY"};
#endif

    for(size_t index = 0U; index < sizeof(errors) / sizeof(errors[0]); index++)
    {
        struct fault_state state = {0, errors[index]};
        int                failures_before;

        failures_before = failures;
        EXPECT(p101_error_has_no_error(err));
        fault_resource_events = 0U;
        errno                 = P101_TEST_ERRNO_SENTINEL;
        p101_env_set_fault_injector(env, fail_next_call, &state);
        int result = p101_tcgetattr(env, err, 0, NULL);
        (void)result;
        EXPECT(state.checks == 1);
        EXPECT(p101_error_is_errno(err, state.code));
        EXPECT(errno == P101_TEST_ERRNO_SENTINEL);
        EXPECT(result == (-1));
        EXPECT(fault_resource_events == 0U);
        write_outcome("p101_tcgetattr", "errno", error_names[index], state.code, failures == failures_before);
        p101_error_reset(err);
    }
    p101_env_set_fault_injector(env, NULL, NULL);
    {
        int   native_status = 0;
        pid_t native_pid    = fork();

        EXPECT(native_pid >= 0);
        if(native_pid == 0)
        {
            struct p101_error *native_err;
            struct p101_env   *native_env;

            (void)alarm(2U);
            (void)unsetenv("P101_CALL_LOG");
            (void)unsetenv("P101_RESOURCE_LOG");
            native_err = p101_error_create(false);
            if(native_err == NULL)
            {
                _Exit(77);
            }
            native_env = p101_env_create(native_err, NULL);
            if(native_env == NULL)
            {
                p101_error_destroy(native_err);
                _Exit(77);
            }
            struct termios native_argument_3 = {0};
            int            native_result     = p101_tcgetattr(native_env, native_err, 0, &native_argument_3);
            (void)native_result;
            p101_env_destroy(native_env);
            p101_error_destroy(native_err);
            _Exit(EXIT_SUCCESS);
        }
        if(native_pid > 0)
        {
            EXPECT(waitpid(native_pid, &native_status, 0) == native_pid);
            EXPECT(WIFEXITED(native_status));
            if(WIFEXITED(native_status))
            {
                EXPECT(WEXITSTATUS(native_status) == EXIT_SUCCESS);
            }
        }
        p101_error_reset(err);
    }
}

/* P101_TEST_CASE(p101_tcgetpgrp) */
static void test_p101_tcgetpgrp(struct p101_env *env, struct p101_error *err)
{
#ifdef __linux__
    static const int         errors[]      = {EBADF, ENOTTY};
    static const char *const error_names[] = {"EBADF", "ENOTTY"};
#elif defined(__APPLE__)
    static const int         errors[]      = {EBADF, ENOTTY};
    static const char *const error_names[] = {"EBADF", "ENOTTY"};
#elif defined(__FreeBSD__)
    static const int         errors[]      = {EBADF, ENOTTY};
    static const char *const error_names[] = {"EBADF", "ENOTTY"};
#else
    static const int         errors[]      = {EBADF, ENOTTY};
    static const char *const error_names[] = {"EBADF", "ENOTTY"};
#endif

    for(size_t index = 0U; index < sizeof(errors) / sizeof(errors[0]); index++)
    {
        struct fault_state state = {0, errors[index]};
        int                failures_before;

        failures_before = failures;
        EXPECT(p101_error_has_no_error(err));
        fault_resource_events = 0U;
        errno                 = P101_TEST_ERRNO_SENTINEL;
        p101_env_set_fault_injector(env, fail_next_call, &state);
        pid_t result = p101_tcgetpgrp(env, err, 0);
        (void)result;
        EXPECT(state.checks == 1);
        EXPECT(p101_error_is_errno(err, state.code));
        EXPECT(errno == P101_TEST_ERRNO_SENTINEL);
        EXPECT(result == (-1));
        EXPECT(fault_resource_events == 0U);
        write_outcome("p101_tcgetpgrp", "errno", error_names[index], state.code, failures == failures_before);
        p101_error_reset(err);
    }
    p101_env_set_fault_injector(env, NULL, NULL);
    {
        int   native_status = 0;
        pid_t native_pid    = fork();

        EXPECT(native_pid >= 0);
        if(native_pid == 0)
        {
            struct p101_error *native_err;
            struct p101_env   *native_env;

            (void)alarm(2U);
            (void)unsetenv("P101_CALL_LOG");
            (void)unsetenv("P101_RESOURCE_LOG");
            native_err = p101_error_create(false);
            if(native_err == NULL)
            {
                _Exit(77);
            }
            native_env = p101_env_create(native_err, NULL);
            if(native_env == NULL)
            {
                p101_error_destroy(native_err);
                _Exit(77);
            }
            pid_t native_result = p101_tcgetpgrp(native_env, native_err, 0);
            (void)native_result;
            p101_env_destroy(native_env);
            p101_error_destroy(native_err);
            _Exit(EXIT_SUCCESS);
        }
        if(native_pid > 0)
        {
            EXPECT(waitpid(native_pid, &native_status, 0) == native_pid);
            EXPECT(WIFEXITED(native_status));
            if(WIFEXITED(native_status))
            {
                EXPECT(WEXITSTATUS(native_status) == EXIT_SUCCESS);
            }
        }
        p101_error_reset(err);
    }
}

/* P101_TEST_CASE(p101_tcgetsid) */
static void test_p101_tcgetsid(struct p101_env *env, struct p101_error *err)
{
#ifdef __linux__
    static const int         errors[]      = {EBADF, ENOTTY};
    static const char *const error_names[] = {"EBADF", "ENOTTY"};
#elif defined(__APPLE__)
    static const int         errors[]      = {EACCES, EBADF, ENOTTY};
    static const char *const error_names[] = {"EACCES", "EBADF", "ENOTTY"};
#elif defined(__FreeBSD__)
    static const int         errors[]      = {EBADF, ENOTTY};
    static const char *const error_names[] = {"EBADF", "ENOTTY"};
#else
    static const int         errors[]      = {EBADF, ENOTTY};
    static const char *const error_names[] = {"EBADF", "ENOTTY"};
#endif

    for(size_t index = 0U; index < sizeof(errors) / sizeof(errors[0]); index++)
    {
        struct fault_state state = {0, errors[index]};
        int                failures_before;

        failures_before = failures;
        EXPECT(p101_error_has_no_error(err));
        fault_resource_events = 0U;
        errno                 = P101_TEST_ERRNO_SENTINEL;
        p101_env_set_fault_injector(env, fail_next_call, &state);
        pid_t result = p101_tcgetsid(env, err, 0);
        (void)result;
        EXPECT(state.checks == 1);
        EXPECT(p101_error_is_errno(err, state.code));
        EXPECT(errno == P101_TEST_ERRNO_SENTINEL);
        EXPECT(result == (-1));
        EXPECT(fault_resource_events == 0U);
        write_outcome("p101_tcgetsid", "errno", error_names[index], state.code, failures == failures_before);
        p101_error_reset(err);
    }
    p101_env_set_fault_injector(env, NULL, NULL);
    {
        int   native_status = 0;
        pid_t native_pid    = fork();

        EXPECT(native_pid >= 0);
        if(native_pid == 0)
        {
            struct p101_error *native_err;
            struct p101_env   *native_env;

            (void)alarm(2U);
            (void)unsetenv("P101_CALL_LOG");
            (void)unsetenv("P101_RESOURCE_LOG");
            native_err = p101_error_create(false);
            if(native_err == NULL)
            {
                _Exit(77);
            }
            native_env = p101_env_create(native_err, NULL);
            if(native_env == NULL)
            {
                p101_error_destroy(native_err);
                _Exit(77);
            }
            pid_t native_result = p101_tcgetsid(native_env, native_err, 0);
            (void)native_result;
            p101_env_destroy(native_env);
            p101_error_destroy(native_err);
            _Exit(EXIT_SUCCESS);
        }
        if(native_pid > 0)
        {
            EXPECT(waitpid(native_pid, &native_status, 0) == native_pid);
            EXPECT(WIFEXITED(native_status));
            if(WIFEXITED(native_status))
            {
                EXPECT(WEXITSTATUS(native_status) == EXIT_SUCCESS);
            }
        }
        p101_error_reset(err);
    }
}

/* P101_TEST_CASE(p101_tcsendbreak) */
static void test_p101_tcsendbreak(struct p101_env *env, struct p101_error *err)
{
#ifdef __linux__
    static const int         errors[]      = {EBADF, EIO, ENOTTY};
    static const char *const error_names[] = {"EBADF", "EIO", "ENOTTY"};
#elif defined(__APPLE__)
    static const int         errors[]      = {EBADF, EIO, ENOTTY};
    static const char *const error_names[] = {"EBADF", "EIO", "ENOTTY"};
#elif defined(__FreeBSD__)
    static const int         errors[]      = {EBADF, EIO, ENOTTY};
    static const char *const error_names[] = {"EBADF", "EIO", "ENOTTY"};
#else
    static const int         errors[]      = {EBADF, EIO, ENOTTY};
    static const char *const error_names[] = {"EBADF", "EIO", "ENOTTY"};
#endif

    for(size_t index = 0U; index < sizeof(errors) / sizeof(errors[0]); index++)
    {
        struct fault_state state = {0, errors[index]};
        int                failures_before;

        failures_before = failures;
        EXPECT(p101_error_has_no_error(err));
        fault_resource_events = 0U;
        errno                 = P101_TEST_ERRNO_SENTINEL;
        p101_env_set_fault_injector(env, fail_next_call, &state);
        int result = p101_tcsendbreak(env, err, 0, 0);
        (void)result;
        EXPECT(state.checks == 1);
        EXPECT(p101_error_is_errno(err, state.code));
        EXPECT(errno == P101_TEST_ERRNO_SENTINEL);
        EXPECT(result == (-1));
        EXPECT(fault_resource_events == 0U);
        write_outcome("p101_tcsendbreak", "errno", error_names[index], state.code, failures == failures_before);
        p101_error_reset(err);
    }
    p101_env_set_fault_injector(env, NULL, NULL);
    {
        int   native_status = 0;
        pid_t native_pid    = fork();

        EXPECT(native_pid >= 0);
        if(native_pid == 0)
        {
            struct p101_error *native_err;
            struct p101_env   *native_env;

            (void)alarm(2U);
            (void)unsetenv("P101_CALL_LOG");
            (void)unsetenv("P101_RESOURCE_LOG");
            native_err = p101_error_create(false);
            if(native_err == NULL)
            {
                _Exit(77);
            }
            native_env = p101_env_create(native_err, NULL);
            if(native_env == NULL)
            {
                p101_error_destroy(native_err);
                _Exit(77);
            }
            int native_result = p101_tcsendbreak(native_env, native_err, 0, 0);
            (void)native_result;
            p101_env_destroy(native_env);
            p101_error_destroy(native_err);
            _Exit(EXIT_SUCCESS);
        }
        if(native_pid > 0)
        {
            EXPECT(waitpid(native_pid, &native_status, 0) == native_pid);
            EXPECT(WIFEXITED(native_status));
            if(WIFEXITED(native_status))
            {
                EXPECT(WEXITSTATUS(native_status) == EXIT_SUCCESS);
            }
        }
        p101_error_reset(err);
    }
}

/* P101_TEST_CASE(p101_tcsetattr) */
static void test_p101_tcsetattr(struct p101_env *env, struct p101_error *err)
{
#ifdef __linux__
    static const int         errors[]      = {EBADF, EINTR, EINVAL, EIO, ENOTTY};
    static const char *const error_names[] = {"EBADF", "EINTR", "EINVAL", "EIO", "ENOTTY"};
#elif defined(__APPLE__)
    static const int         errors[]      = {EBADF, EINTR, EINVAL, EIO, ENOTTY};
    static const char *const error_names[] = {"EBADF", "EINTR", "EINVAL", "EIO", "ENOTTY"};
#elif defined(__FreeBSD__)
    static const int         errors[]      = {EBADF, EINTR, EINVAL, EIO, ENOTTY};
    static const char *const error_names[] = {"EBADF", "EINTR", "EINVAL", "EIO", "ENOTTY"};
#else
    static const int         errors[]      = {EBADF, EINTR, EINVAL, EIO, ENOTTY};
    static const char *const error_names[] = {"EBADF", "EINTR", "EINVAL", "EIO", "ENOTTY"};
#endif

    for(size_t index = 0U; index < sizeof(errors) / sizeof(errors[0]); index++)
    {
        struct fault_state state = {0, errors[index]};
        int                failures_before;

        failures_before = failures;
        EXPECT(p101_error_has_no_error(err));
        fault_resource_events = 0U;
        errno                 = P101_TEST_ERRNO_SENTINEL;
        p101_env_set_fault_injector(env, fail_next_call, &state);
        int result = p101_tcsetattr(env, err, 0, 0, NULL);
        (void)result;
        EXPECT(state.checks == 1);
        EXPECT(p101_error_is_errno(err, state.code));
        EXPECT(errno == P101_TEST_ERRNO_SENTINEL);
        EXPECT(result == (-1));
        EXPECT(fault_resource_events == 0U);
        write_outcome("p101_tcsetattr", "errno", error_names[index], state.code, failures == failures_before);
        p101_error_reset(err);
    }
    p101_env_set_fault_injector(env, NULL, NULL);
    {
        int   native_status = 0;
        pid_t native_pid    = fork();

        EXPECT(native_pid >= 0);
        if(native_pid == 0)
        {
            struct p101_error *native_err;
            struct p101_env   *native_env;

            (void)alarm(2U);
            (void)unsetenv("P101_CALL_LOG");
            (void)unsetenv("P101_RESOURCE_LOG");
            native_err = p101_error_create(false);
            if(native_err == NULL)
            {
                _Exit(77);
            }
            native_env = p101_env_create(native_err, NULL);
            if(native_env == NULL)
            {
                p101_error_destroy(native_err);
                _Exit(77);
            }
            struct termios native_argument_4 = {0};
            int            native_result     = p101_tcsetattr(native_env, native_err, 0, 0, &native_argument_4);
            (void)native_result;
            p101_env_destroy(native_env);
            p101_error_destroy(native_err);
            _Exit(EXIT_SUCCESS);
        }
        if(native_pid > 0)
        {
            EXPECT(waitpid(native_pid, &native_status, 0) == native_pid);
            EXPECT(WIFEXITED(native_status));
            if(WIFEXITED(native_status))
            {
                EXPECT(WEXITSTATUS(native_status) == EXIT_SUCCESS);
            }
        }
        p101_error_reset(err);
    }
}

/* P101_TEST_CASE(p101_tcsetpgrp) */
static void test_p101_tcsetpgrp(struct p101_env *env, struct p101_error *err)
{
#ifdef __linux__
    static const int         errors[]      = {EBADF, EINVAL, ENOTTY, EPERM};
    static const char *const error_names[] = {"EBADF", "EINVAL", "ENOTTY", "EPERM"};
#elif defined(__APPLE__)
    static const int         errors[]      = {EBADF, EINVAL, ENOTTY, EPERM};
    static const char *const error_names[] = {"EBADF", "EINVAL", "ENOTTY", "EPERM"};
#elif defined(__FreeBSD__)
    static const int         errors[]      = {EBADF, EINVAL, ENOTTY, EPERM};
    static const char *const error_names[] = {"EBADF", "EINVAL", "ENOTTY", "EPERM"};
#else
    static const int         errors[]      = {EBADF, EINTR, EINVAL, EIO, ENOTTY, EPERM};
    static const char *const error_names[] = {"EBADF", "EINTR", "EINVAL", "EIO", "ENOTTY", "EPERM"};
#endif

    for(size_t index = 0U; index < sizeof(errors) / sizeof(errors[0]); index++)
    {
        struct fault_state state = {0, errors[index]};
        int                failures_before;

        failures_before = failures;
        EXPECT(p101_error_has_no_error(err));
        fault_resource_events = 0U;
        errno                 = P101_TEST_ERRNO_SENTINEL;
        p101_env_set_fault_injector(env, fail_next_call, &state);
        int result = p101_tcsetpgrp(env, err, 0, 0);
        (void)result;
        EXPECT(state.checks == 1);
        EXPECT(p101_error_is_errno(err, state.code));
        EXPECT(errno == P101_TEST_ERRNO_SENTINEL);
        EXPECT(result == (-1));
        EXPECT(fault_resource_events == 0U);
        write_outcome("p101_tcsetpgrp", "errno", error_names[index], state.code, failures == failures_before);
        p101_error_reset(err);
    }
    p101_env_set_fault_injector(env, NULL, NULL);
    {
        int   native_status = 0;
        pid_t native_pid    = fork();

        EXPECT(native_pid >= 0);
        if(native_pid == 0)
        {
            struct p101_error *native_err;
            struct p101_env   *native_env;

            (void)alarm(2U);
            (void)unsetenv("P101_CALL_LOG");
            (void)unsetenv("P101_RESOURCE_LOG");
            native_err = p101_error_create(false);
            if(native_err == NULL)
            {
                _Exit(77);
            }
            native_env = p101_env_create(native_err, NULL);
            if(native_env == NULL)
            {
                p101_error_destroy(native_err);
                _Exit(77);
            }
            int native_result = p101_tcsetpgrp(native_env, native_err, 0, 0);
            (void)native_result;
            p101_env_destroy(native_env);
            p101_error_destroy(native_err);
            _Exit(EXIT_SUCCESS);
        }
        if(native_pid > 0)
        {
            EXPECT(waitpid(native_pid, &native_status, 0) == native_pid);
            EXPECT(WIFEXITED(native_status));
            if(WIFEXITED(native_status))
            {
                EXPECT(WEXITSTATUS(native_status) == EXIT_SUCCESS);
            }
        }
        p101_error_reset(err);
    }
}

/* P101_TEST_CASE(p101_ttyname_r) */
static void test_p101_ttyname_r(struct p101_env *env, struct p101_error *err)
{
    char          argument_3[4];
    unsigned char argument_3_before[sizeof(argument_3)];
    memset(argument_3, 0xA5, sizeof(argument_3));
    memcpy(argument_3_before, argument_3, sizeof(argument_3));
#ifdef __linux__
    static const int         errors[]      = {EBADF, ENODEV, ENOTTY, ERANGE};
    static const char *const error_names[] = {"EBADF", "ENODEV", "ENOTTY", "ERANGE"};
#elif defined(__APPLE__)
    static const int         errors[]      = {EBADF, ENOTTY, ERANGE};
    static const char *const error_names[] = {"EBADF", "ENOTTY", "ERANGE"};
#elif defined(__FreeBSD__)
    static const int         errors[]      = {EBADF, ENOTTY, ERANGE};
    static const char *const error_names[] = {"EBADF", "ENOTTY", "ERANGE"};
#else
    static const int         errors[]      = {EBADF, ENOTTY, ERANGE};
    static const char *const error_names[] = {"EBADF", "ENOTTY", "ERANGE"};
#endif

    for(size_t index = 0U; index < sizeof(errors) / sizeof(errors[0]); index++)
    {
        struct fault_state state = {0, errors[index]};
        int                failures_before;

        failures_before = failures;
        EXPECT(p101_error_has_no_error(err));
        fault_resource_events = 0U;
        errno                 = P101_TEST_ERRNO_SENTINEL;
        p101_env_set_fault_injector(env, fail_next_call, &state);
        int result = p101_ttyname_r(env, err, 0, argument_3, 0);
        (void)result;
        EXPECT(state.checks == 1);
        EXPECT(p101_error_is_errno(err, state.code));
        EXPECT(errno == P101_TEST_ERRNO_SENTINEL);
        EXPECT(result == state.code);
        EXPECT(memcmp(argument_3, argument_3_before, sizeof(argument_3)) == 0);
        EXPECT(fault_resource_events == 0U);
        write_outcome("p101_ttyname_r", "errno", error_names[index], state.code, failures == failures_before);
        p101_error_reset(err);
    }
    p101_env_set_fault_injector(env, NULL, NULL);
    {
        int   native_status = 0;
        pid_t native_pid    = fork();

        EXPECT(native_pid >= 0);
        if(native_pid == 0)
        {
            struct p101_error *native_err;
            struct p101_env   *native_env;

            (void)alarm(2U);
            (void)unsetenv("P101_CALL_LOG");
            (void)unsetenv("P101_RESOURCE_LOG");
            native_err = p101_error_create(false);
            if(native_err == NULL)
            {
                _Exit(77);
            }
            native_env = p101_env_create(native_err, NULL);
            if(native_env == NULL)
            {
                p101_error_destroy(native_err);
                _Exit(77);
            }
            char native_argument_3[PATH_MAX] = {0};
            int  native_result               = p101_ttyname_r(native_env, native_err, 0, native_argument_3, 0);
            (void)native_result;
            p101_env_destroy(native_env);
            p101_error_destroy(native_err);
            _Exit(EXIT_SUCCESS);
        }
        if(native_pid > 0)
        {
            EXPECT(waitpid(native_pid, &native_status, 0) == native_pid);
            EXPECT(WIFEXITED(native_status));
            if(WIFEXITED(native_status))
            {
                EXPECT(WEXITSTATUS(native_status) == EXIT_SUCCESS);
            }
        }
        p101_error_reset(err);
    }
}

/* P101_TEST_CASE(p101_unlockpt) */
static void test_p101_unlockpt(struct p101_env *env, struct p101_error *err)
{
#ifdef __linux__
    static const int         errors[]      = {EBADF, EINVAL};
    static const char *const error_names[] = {"EBADF", "EINVAL"};
#elif defined(__APPLE__)
    static const int         errors[]      = {EBADF, EFAULT, EINVAL, EIO, EOVERFLOW, ERANGE};
    static const char *const error_names[] = {"EBADF", "EFAULT", "EINVAL", "EIO", "EOVERFLOW", "ERANGE"};
#elif defined(__FreeBSD__)
    static const int         errors[]      = {EBADF, EINVAL};
    static const char *const error_names[] = {"EBADF", "EINVAL"};
#else
    static const int         errors[]      = {EBADF, EINVAL};
    static const char *const error_names[] = {"EBADF", "EINVAL"};
#endif

    for(size_t index = 0U; index < sizeof(errors) / sizeof(errors[0]); index++)
    {
        struct fault_state state = {0, errors[index]};
        int                failures_before;

        failures_before = failures;
        EXPECT(p101_error_has_no_error(err));
        fault_resource_events = 0U;
        errno                 = P101_TEST_ERRNO_SENTINEL;
        p101_env_set_fault_injector(env, fail_next_call, &state);
        int result = p101_unlockpt(env, err, 0);
        (void)result;
        EXPECT(state.checks == 1);
        EXPECT(p101_error_is_errno(err, state.code));
        EXPECT(errno == P101_TEST_ERRNO_SENTINEL);
        EXPECT(result == (-1));
        EXPECT(fault_resource_events == 0U);
        write_outcome("p101_unlockpt", "errno", error_names[index], state.code, failures == failures_before);
        p101_error_reset(err);
    }
    p101_env_set_fault_injector(env, NULL, NULL);
    {
        int   native_status = 0;
        pid_t native_pid    = fork();

        EXPECT(native_pid >= 0);
        if(native_pid == 0)
        {
            struct p101_error *native_err;
            struct p101_env   *native_env;

            (void)alarm(2U);
            (void)unsetenv("P101_CALL_LOG");
            (void)unsetenv("P101_RESOURCE_LOG");
            native_err = p101_error_create(false);
            if(native_err == NULL)
            {
                _Exit(77);
            }
            native_env = p101_env_create(native_err, NULL);
            if(native_env == NULL)
            {
                p101_error_destroy(native_err);
                _Exit(77);
            }
            int native_result = p101_unlockpt(native_env, native_err, 0);
            (void)native_result;
            p101_env_destroy(native_env);
            p101_error_destroy(native_err);
            _Exit(EXIT_SUCCESS);
        }
        if(native_pid > 0)
        {
            EXPECT(waitpid(native_pid, &native_status, 0) == native_pid);
            EXPECT(WIFEXITED(native_status));
            if(WIFEXITED(native_status))
            {
                EXPECT(WEXITSTATUS(native_status) == EXIT_SUCCESS);
            }
        }
        p101_error_reset(err);
    }
}

int main(void)
{
    const char        *outcome_path;
    struct p101_error *err;
    struct p101_env   *env;

    outcome_path = getenv("P101_WRAPPER_OUTCOME_LOG");
    if(outcome_path != NULL && outcome_path[0] != '\0')
    {
        outcome_stream = fopen(outcome_path, "a");
        if(outcome_stream == NULL)
        {
            fprintf(stderr, "FAIL: cannot open wrapper outcome receipt\n");
            return EXIT_FAILURE;
        }
    }
    err = p101_error_create(false);
    if(err == NULL)
    {
        if(outcome_stream != NULL)
        {
            (void)fclose(outcome_stream);
        }
        return EXIT_FAILURE;
    }
    env = p101_env_create(err, NULL);
    if(env == NULL)
    {
        p101_error_destroy(err);
        if(outcome_stream != NULL)
        {
            (void)fclose(outcome_stream);
        }
        return EXIT_FAILURE;
    }
    p101_env_set_fd_observer(env, count_fd_event, NULL);
    p101_env_set_alloc_observer(env, count_alloc_event, NULL);
    p101_env_set_resource_observer(env, count_resource_event, NULL);
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
    if(outcome_stream != NULL && fclose(outcome_stream) != 0)
    {
        fprintf(stderr, "FAIL: cannot close wrapper outcome receipt\n");
        failures++;
    }
    return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
