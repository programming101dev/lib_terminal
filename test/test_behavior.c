#include <limits.h>
#include <p101_env/env.h>
#include <p101_error/error.h>
#include <p101_terminal/terminal.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>

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

int main(void)
{
    struct p101_error *err;
    struct p101_env   *env;
    struct termios     settings = {0};
    struct termios     expected = {0};

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

    /* P101_TEST_CASE(p101_cfgetispeed) */
    EXPECT(p101_cfgetispeed(env, &settings) == cfgetispeed(&settings));
    /* P101_TEST_CASE(p101_cfgetospeed) */
    EXPECT(p101_cfgetospeed(env, &settings) == cfgetospeed(&settings));

    settings.c_iflag = UINT_MAX;
    settings.c_oflag = UINT_MAX;
    settings.c_cflag = UINT_MAX;
    settings.c_lflag = UINT_MAX;
    expected         = settings;
    cfmakeraw(&expected);
    /* P101_TEST_CASE(p101_cfmakeraw) */
    p101_cfmakeraw(env, &settings);
    EXPECT(settings.c_iflag == expected.c_iflag);
    EXPECT(settings.c_oflag == expected.c_oflag);
    EXPECT(settings.c_cflag == expected.c_cflag);
    EXPECT(settings.c_lflag == expected.c_lflag);

    p101_env_destroy(env);
    p101_error_destroy(err);
    return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
