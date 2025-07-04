#ifndef DEBUG_H
#define DEBUG_H

// Uncomment the following line to enable debug prints:
// #define DEBUG_ENABLED

#ifdef DEBUG_ENABLED
    #define DEBUG_PRINT(fmt, ...) \
        printf("[DEBUG] " fmt "\n", ##__VA_ARGS__)
#else
    #define DEBUG_PRINT(fmt, ...) \
        do { } while (0)
#endif

#endif // DEBUG_H