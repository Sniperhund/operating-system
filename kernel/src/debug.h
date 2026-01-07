#pragma once

#define DO_INIT(msg, func) \
    do { \
        int status = (func); \
        printf("%s: %s\n", msg, status == 0 ? "DONE" : "FAILED"); \
    } while (0)