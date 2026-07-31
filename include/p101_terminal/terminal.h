#ifndef LIBP101_TERMINAL_TERMINAL_H
#define LIBP101_TERMINAL_TERMINAL_H

/*
 * Copyright 2026 D'Arcy Smith.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 */

#include <p101_env/env.h>
#include <p101_error/attributes.h>
#include <termios.h>
#include <unistd.h>

#ifdef __cplusplus
extern "C"
{
#endif

    speed_t p101_cfgetispeed(const struct p101_env *env, const struct termios *termios_p);
    speed_t p101_cfgetospeed(const struct p101_env *env, const struct termios *termios_p);
    void    p101_cfmakeraw(const struct p101_env *env, struct termios *termios_p);
    int     p101_cfsetispeed(const struct p101_env *env, struct p101_error *err, struct termios *termios_p, speed_t speed);
    int     p101_cfsetospeed(const struct p101_env *env, struct p101_error *err, struct termios *termios_p, speed_t speed);
    int     p101_cfsetspeed(const struct p101_env *env, struct p101_error *err, struct termios *termios_p, speed_t speed);
    int     p101_grantpt(const struct p101_env *env, struct p101_error *err, int fildes);
    int     p101_isatty(const struct p101_env *env, struct p101_error *err, int fildes);
    int     p101_posix_openpt(const struct p101_env *env, struct p101_error *err, int oflag) P101_ATTR_WARN_UNUSED_RESULT;
    char   *p101_ptsname(const struct p101_env *env, struct p101_error *err, int fildes);
    int     p101_tcdrain(const struct p101_env *env, struct p101_error *err, int fildes);
    int     p101_tcflow(const struct p101_env *env, struct p101_error *err, int fildes, int action);
    int     p101_tcflush(const struct p101_env *env, struct p101_error *err, int fildes, int queue_selector);
    int     p101_tcgetattr(const struct p101_env *env, struct p101_error *err, int fildes, struct termios *termios_p);
    pid_t   p101_tcgetpgrp(const struct p101_env *env, struct p101_error *err, int fildes);
    pid_t   p101_tcgetsid(const struct p101_env *env, struct p101_error *err, int fildes);
    int     p101_tcsendbreak(const struct p101_env *env, struct p101_error *err, int fildes, int duration);
    int     p101_tcsetattr(const struct p101_env *env, struct p101_error *err, int fildes, int optional_actions, const struct termios *termios_p);
    int     p101_tcsetpgrp(const struct p101_env *env, struct p101_error *err, int fildes, pid_t pgid_id);
    int     p101_ttyname_r(const struct p101_env *env, struct p101_error *err, int fildes, char *name, size_t namesize);
    int     p101_unlockpt(const struct p101_env *env, struct p101_error *err, int fildes);

#ifdef __cplusplus
}
#endif

#endif    // LIBP101_TERMINAL_TERMINAL_H
