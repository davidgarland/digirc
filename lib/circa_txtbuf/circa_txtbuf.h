/*
** circa_txtbuf.h | The Circa Library Set | In-place text buffers for C.
** https://github.com/davidgarland/circa_txt_buf
*/

#ifndef CIRCA_TXTBUF_H
#define CIRCA_TXTBUF_H

/*
** Dependencies
*/

#include <string.h>
#include <stdarg.h>
#include <stdio.h>

#include <circa_core.h>

/*
** Type Definitions
*/

typedef struct {
  size_t cap;
  size_t len;
  char *raw;
} TxtBuf;

/*
** Traversals
*/

#define txtbuf_foreach(X, XS) \
for (size_t I = 0, J = 0; I < (XS)->len; I++, J = 0) \
for (char X; (txtbuf_get(XS, I, &X), J != 1); J = 1)

/*
** Forward Declarations
*/

CIRCA CE txtbuf_require(TxtBuf *tb, size_t cap);

/*
** Accessors
*/

CIRCA
CE txtbuf_set(TxtBuf *tb, size_t a, char v) {
  CE_CHECK(!tb, CE_NULL_ARG);
  CE_CHECK(!tb->raw, CE_NULL_ARG);
  CE_CHECK(a > tb->len, CE_OOB);

  if (a == tb->len) {
    if (v == '\0')
      return CE_OK;
    CE req_fail = txtbuf_require(tb, a + 2);
    if (req_fail)
      return req_fail;
    tb->raw[a + 1] = '\0';
    tb->len = a + 1;
  } else if (v == '\0') {
    return CE_ZERO_ARG;
  }
  
  tb->raw[a] = v;
 
  return CE_OK;
}

CIRCA
CE txtbuf_get_by_cap(TxtBuf *tb, size_t a, char *r) {
  CE_CHECK(!tb, CE_NULL_ARG);
  CE_CHECK(!tb->raw, CE_NULL_ARG);
  CE_CHECK(a >= tb->cap, CE_OOB);
  *r = tb->raw[a];
  return CE_OK;
}

CIRCA
CE txtbuf_get(TxtBuf *tb, size_t a, char *r) {
  CE_CHECK(!tb, CE_NULL_ARG);
  CE_CHECK(!tb->raw, CE_NULL_ARG);
  CE_CHECK(a >= tb->len, CE_OOB);
  *r = tb->raw[a];
  return CE_OK;
}

CIRCA
CE txtbuf_push(TxtBuf *tb, char c) {
  return txtbuf_set(tb, tb->len, c);
}

CIRCA
CE txtbuf_pop(TxtBuf *tb, char *r) {
  CE_CHECK(!tb, CE_NULL_ARG);
  CE_CHECK(!tb->raw, CE_NULL_ARG);
  char c;
  if (tb->len) {
    c = tb->raw[--tb->len];
    tb->raw[tb->len] = '\0';
  } else {
    c = '\0';
  }
  if (r)
    *r = c;
  return CE_OK;
}

/*
** Allocators
*/

CIRCA
CE txtbuf_alloc(TxtBuf *tb, size_t cap) {
  CE_CHECK(!tb, CE_NULL_ARG);
  CE_CHECK(!cap, CE_ZERO_ARG);
  if (tb->raw) {
    tb->raw = realloc(tb->raw, cap);
    CE_CRITICAL(!tb->raw, CE_REALLOC);
    memset(tb->raw, 0, cap);
  } else {
    tb->raw = calloc(cap, 1);
    CE_CRITICAL(!tb->raw, CE_MALLOC);
  }
  tb->cap = cap;
  tb->len = 0;
  return CE_OK;
}

CIRCA
CE txtbuf_realloc(TxtBuf *tb, size_t cap) {
  CE_CHECK(!tb, CE_NULL_ARG);
  CE_CHECK(!tb->raw, CE_NULL_ARG);
  CE_CHECK(!cap, CE_ZERO_ARG);
  tb->raw = realloc(tb->raw, cap);
  CE_CRITICAL(!tb->raw, CE_REALLOC);
  if (cap > tb->cap)
    memset(tb->raw + tb->cap, 0, cap - tb->cap);
  tb->cap = cap;
  if (cap < tb->len)
    tb->len = cap - 1;
  return CE_OK;
}

CIRCA
CE txtbuf_require(TxtBuf *tb, size_t cap) {
  CE_CHECK(!tb, CE_NULL_ARG);
  CE_CHECK(!tb->raw, CE_NULL_ARG);
  if (cap <= tb->cap)
    return CE_OK;
  return txtbuf_realloc(tb, cap);
}

CIRCA
CE txtbuf_free(TxtBuf *tb) {
  CE_CHECK(!tb, CE_NULL_ARG);
  if (tb->raw) {
    tb->cap = 0;
    tb->len = 0;
    free(tb->raw);
    tb->raw = NULL;
  }
}

/*
** Copy Operations
*/

CIRCA
CE txtbuf_clear(TxtBuf *tb) {
  CE_CHECK(!tb, CE_NULL_ARG);
  CE_CHECK(!tb->raw, CE_NULL_ARG);
  tb->len = 0;
  tb->raw[0] = '\0';
  return CE_OK;
}

CIRCA
CE txtbuf_cpy(TxtBuf *dst, TxtBuf *src) {
  CE_CHECK(!dst, CE_NULL_ARG);
  CE_CHECK(!dst->raw, CE_NULL_ARG);
  CE_CHECK(!src, CE_NULL_ARG);
  CE_CHECK(!src->raw, CE_NULL_ARG);
  size_t len = src->len;
  CE req_fail = txtbuf_require(dst, len);
  if (req_fail)
    return req_fail;
  memcpy(dst->raw, src, len);
  dst->len = len;
  return CE_OK;
}

CIRCA
CE txtbuf_cpy_slice(TxtBuf *dst, TxtBuf *src, Slice s) {
  CE_CHECK(!dst, CE_NULL_ARG);
  CE_CHECK(!dst->raw, CE_NULL_ARG);
  CE_CHECK(!src, CE_NULL_ARG);
  CE_CHECK(!src->raw, CE_NULL_ARG);
  CE_GUARD (1)
    if (!slice_contains(s, (Slice) {0, src->len}))
      return CE_OOB;

  size_t s_len = slice_len(s);

  CE req_fail = txtbuf_require(dst, s_len + 1);
  if (req_fail)
    return req_fail;

  memcpy(dst->raw, src->raw + s.l, s_len);
  dst->len = s_len;
  dst->raw[s_len] = '\0';

  return CE_OK;
}

CIRCA
CE txtbuf_cpy_cstr(TxtBuf *dst, char *src) {
  CE_CHECK(!dst, CE_NULL_ARG);
  CE_CHECK(!dst->raw, CE_NULL_ARG);
  CE_CHECK(!src, CE_NULL_ARG);
  size_t len = strlen(src);
  CE req_fail = txtbuf_require(dst, len + 1);
  if (req_fail)
    return req_fail;
  memcpy(dst->raw, src, len);
  dst->len = len;
  return CE_OK;
}

CIRCA
CE txtbuf_cpy_cstr_slice(TxtBuf *dst, char *src, Slice s) {
  CE_CHECK(!dst, CE_NULL_ARG);
  CE_CHECK(!dst->raw, CE_NULL_ARG);
  CE_CHECK(!src, CE_NULL_ARG);
  CE_GUARD (1)
    if (!slice_contains(s, (Slice) {0, strlen(src)}))
      return CE_OOB;

  size_t s_len = slice_len(s);

  CE req_fail = txtbuf_require(dst, s_len + 1);
  if (req_fail)
    return req_fail;

  memcpy(dst->raw, src + s.l, s_len);
  dst->len = s_len;
  dst->raw[s_len] = '\0';

  return CE_OK;
}

/*
** Formatting Operations
*/

CIRCA CIRCA_VPRINTF(2)
CE txtbuf_fmt_va(TxtBuf *tb, const char *fmt, va_list ap) {
  CE_CHECK(!tb, CE_NULL_ARG);
  CE_CHECK(!tb->raw, CE_NULL_ARG);
  CE_CHECK(!fmt, CE_NULL_ARG);
  va_list ap2;
  va_copy(ap2, ap);
  size_t len = vsnprintf(NULL, 0, fmt, ap2);
  va_end(ap2);
  CE req_fail = txtbuf_require(tb, len + 1);
  if (req_fail)
    return req_fail;
  size_t written = vsnprintf(tb->raw, len + 1, fmt, ap);
  if (written != len)
    return CE_FMT;
  tb->len = len;
  return CE_OK;
}

CIRCA CIRCA_PRINTF(2)
CE txtbuf_fmt(TxtBuf *tb, const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  CE fmt_fail = txtbuf_fmt_va(tb, fmt, ap);
  va_end(ap);
  if (fmt_fail)
    return fmt_fail;
  return CE_OK;
}

#endif // CIRCA_TXTBUF_H
