#ifndef UTILS_H
#define UTILS_H

// LogError - helper function for error handling.
inline void log_error(const char *str) {
    fprintf(stderr, "Log error: %s\n", str);
}

#endif
