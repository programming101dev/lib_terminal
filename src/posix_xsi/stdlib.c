#include "p101_terminal/terminal.h"
#include <p101_c/p101_string.h>
#include <p101_env/wrapper.h>
#include <stdlib.h>
#include <string.h>

int p101_grantpt(const struct p101_env *env, struct p101_error *err, int fildes)
{
    int ret_val;

    P101_TRACE(env);
    P101_WRAPPER_FAULT_RETURN(env, err, -1);
    errno   = 0;
    ret_val = grantpt(fildes);

    if(ret_val == -1)
    {
        P101_ERROR_RAISE_ERRNO(err, errno);
    }

    P101_TRACE_EXIT(env);
    return ret_val;
}

int p101_posix_openpt(const struct p101_env *env, struct p101_error *err, int oflag)
{
    int ret_val;

    P101_TRACE(env);
    P101_WRAPPER_FAULT_RETURN(env, err, -1);
    errno   = 0;
    ret_val = posix_openpt(oflag);

    if(ret_val == -1)
    {
        P101_ERROR_RAISE_ERRNO(err, errno);
    }
    else
    {
        P101_TRACK_OPEN(env, ret_val);
    }

    P101_TRACE_EXIT(env);

    return ret_val;
}

char *p101_ptsname(const struct p101_env *env, struct p101_error *err, int fildes)
{
    char *ret_val;

    P101_TRACE(env);
    P101_WRAPPER_FAULT_RETURN(env, err, NULL);
    errno   = 0;
    ret_val = ptsname(fildes);

    if(ret_val == NULL)
    {
        P101_ERROR_RAISE_ERRNO(err, (errno == 0) ? EIO : errno);
    }

    P101_TRACE_EXIT(env);
    return ret_val;
}

int p101_unlockpt(const struct p101_env *env, struct p101_error *err, int fildes)
{
    int ret_val;

    P101_TRACE(env);
    P101_WRAPPER_FAULT_RETURN(env, err, -1);
    errno   = 0;
    ret_val = unlockpt(fildes);

    if(ret_val == -1)
    {
        P101_ERROR_RAISE_ERRNO(err, errno);
    }

    P101_TRACE_EXIT(env);
    return ret_val;
}
