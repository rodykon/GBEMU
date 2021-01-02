#ifndef LOG__
#define LOG__

#include <stdio.h>

#ifdef DEBUG
#define log(fmt, ...) fprintf(stderr, "%s:%d:%s(): " fmt "\n", __FILE__, __LINE__, __func__, ##__VA_ARGS__)
#else
#define log(fmt, ...) {}
#endif

#endif
