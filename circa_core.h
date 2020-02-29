/*
** circa_core.h | The Circa Library Set | Core header w/ utils and whatnot.
** https://github.com/davidgarland/circa_core
*/

#ifndef CIRCA_CORE_H
#define CIRCA_CORE_H

/*
** Dependencies
*/

#include <stdbool.h>
#include <stdlib.h>

/*
** Type Definitions
*/

typedef enum {
  CE_OK,
  CE_FMT,
  CE_OOB,
  CE_NULL_ARG,
  CE_ZERO_ARG,
  CE_MALLOC,
  CE_REALLOC,
  CE_LENGTH
} CE;

typedef struct {
  size_t l;
  size_t r;
} Slice;

/*
** Constants
*/

static const char *CE_EXPLAIN[CE_LENGTH] = {
  [CE_OK] = "OK.",
  [CE_FMT] = "An internal call to vsnprintf failed",
  [CE_OOB] = "Index out of bounds",
  [CE_NULL_ARG] = "Null argument error",
  [CE_ZERO_ARG] = "Zero argument error; likely capacity or arithmetic",
  [CE_MALLOC] = "An internal call to malloc/calloc failed",
  [CE_REALLOC] = "An internal call to realloc failed"
};

/*
** Core Macros
*/

#ifdef __GNUC__
  #define CIRCA_GNU(...) __VA_ARGS__
#else
  #define CIRCA_GNU(...)
#endif

#define CIRCA_ATTR(...) CIRCA_GNU(__attribute__((__VA_ARGS__)))

#define CIRCA_VPRINTF(X) CIRCA_ATTR(__format__(__printf__, (X), 0))
#define CIRCA_PRINTF(X) __attribute__((__format__(__printf__, (X), (X + 1))))

#define CIRCA static inline

#define CIRCA_RETURNS CIRCA CIRCA_ATTR(warn_unused_result)

/*
** Debug Macros
*/

#ifdef NDEBUG
  #define CE_GUARD(...) if (__VA_ARGS__)
#else
  #define CE_GUARD(...) if (0)
#endif

#ifdef CIRCA_LOGGING
  #define CE_LOG_(FILE, FUNC, LINE, FMT, ...) \
    (printf("[circa] %s: in %s on line %zu:\n", FILE, FUNC, (size_t) LINE), printf(FMT, __VA_ARGS__))
#else
  #define CE_LOG_(FILE, FUNC, LINE, FMT, ...) (0)
#endif

#define CE_LOG(FMT, ...) CE_LOG_(__FILE__, __func__, __LINE__, FMT, __VA_ARGS__)

#define CE_COMPLAIN(E) CE_LOG("%s", CE_EXPLAIN[E])

#define CE_REPORT(E, P) (CE_COMPLAIN(E), P ? (*(P) = E) : 0)

#define CE_THROW(E) return (CE_COMPLAIN(E), E)

#define CE_CHECK(C, E) CE_GUARD(C) CE_THROW(E)

#define CE_CRITICAL(C, E) if (C) CE_THROW(E)

/*
** Slice Functions
*/

CIRCA
size_t slice_len(Slice s) {
  return s.r - s.l + 1;
}

CIRCA
Slice slice_combine(Slice a, Slice b) {
  return (Slice) {a.l < b.l ? a.l : b.l, a.r > b.r ? a.r : b.r};
}

CIRCA
bool slice_contains(Slice a, Slice b) {
  return (a.l <= b.l) && (a.r >= b.r);
}

#endif // CIRCA_CORE_H
