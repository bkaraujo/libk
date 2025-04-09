#include "libk/time.h"
#include "libk/logger.h"

static KLogLevel m_level = K_LOG_LEVEL_INFO;
static const char *strings[] = {"VERBOSE ", "TRACE  ", "DEBUG  ", "INFO   ", "WARN   ", "ERROR  ", "FATAL  "};

#if defined(K_PLATFORM_LINUX)
static const char *colors[] = { "\033[1;37m" , "\033[1;36m", "\033[1;34m", "\033[1;32m", "\033[1;33m", "\033[1;31m", "\033[1;31m" };
#endif

void k_logger_loglevel(const KLogLevel desired){
    m_level = desired;
}

void k_logger_write(const KLogLevel level, const char *filename, const u32 lineno, const char *message, ...) {
    if (level < m_level) { return; }

    char output[2048] = { 0 };
    va_list arg_ptr;
    va_start(arg_ptr, message);
    vsnprintf(output, 2048, message, arg_ptr);
    va_end(arg_ptr);

    u16 index = 0;
    const char* character = filename;
    for (u16 i = 0; *character != '\0' ; ++character) {
        if (*character == '/') { index = i + 1; }
        if (i++ == U32_MAX) { i = 0; }
    }

    KClock clock; k_time_clock(&clock);
    fprintf(stdout, "%s%d-%02d-%02d %02d:%02d:%02d,%06u %lu %20s:%04d %s %s\n\033[1;30m",
        colors[level],
        clock.year, clock.month, clock.day,
        clock.hour, clock.minute, clock.second, clock.millis,
        thrd_current(),
        filename + index, lineno,
        strings[level],
        output
    );

    fflush(stdout);
// #if ! defined(BKS_BUILD_RELEASE)
//     // Print the stack trace
//     if (level == K_LOG_LEVEL_FATAL) {
//         const char *format = "%66s at %20s:%04d %s(%s)\n\033[1;30m";
//
//         fprintf(stdout, "\n");
//         for (u8 i = stack_index ; i > 0 ; --i) {
//             fprintf(stdout, format, colors[level], stack[i].filename, stack[i].lineno, stack[i].function, stack[i].arguments);
//         }
//
//         fprintf(stdout, format, colors[level], stack[0].filename, stack[0].lineno, stack[0].function, stack[0].arguments);
//     }
// #endif
}