#ifndef LIBP101_TERMINAL_P101_TERMIOS_H
#define LIBP101_TERMINAL_P101_TERMIOS_H

/*
 * Copyright 2026 D'Arcy Smith.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef LIBP101_TERMINAL_SHARED_DECLARATIONS
    #define LIBP101_TERMINAL_SHARED_DECLARATIONS
    #include <p101_env/env.h>
    #include <p101_error/attributes.h>
    #include <termios.h>
    #include <unistd.h>
#endif    // LIBP101_TERMINAL_SHARED_DECLARATIONS

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
    int     p101_tcdrain(const struct p101_env *env, struct p101_error *err, int fildes);
    int     p101_tcflow(const struct p101_env *env, struct p101_error *err, int fildes, int action);
    int     p101_tcflush(const struct p101_env *env, struct p101_error *err, int fildes, int queue_selector);
    int     p101_tcgetattr(const struct p101_env *env, struct p101_error *err, int fildes, struct termios *termios_p);
    pid_t   p101_tcgetsid(const struct p101_env *env, struct p101_error *err, int fildes);
    int     p101_tcsendbreak(const struct p101_env *env, struct p101_error *err, int fildes, int duration);
    int     p101_tcsetattr(const struct p101_env *env, struct p101_error *err, int fildes, int optional_actions, const struct termios *termios_p);

#ifdef __cplusplus
}
#endif

#endif    // LIBP101_TERMINAL_P101_TERMIOS_H
