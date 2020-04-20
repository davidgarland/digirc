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
  char *data;
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

/* Allocators */

CIRCA TxtBuf txtbuf_init(void);
CIRCA CE txtbuf_alloc(TxtBuf *tb, size_t cap);
CIRCA CE txtbuf_realloc(TxtBuf *tb, size_t cap);
CIRCA CE txtbuf_prealloc(TxtBuf *tb, size_t cap);
CIRCA CE txtbuf_free(TxtBuf *tb);

/* Accessors */

CIRCA CE txtbuf_set(TxtBuf *tb, size_t a, char v);
CIRCA CE txtbuf_get(TxtBuf *tb, size_t a, char *r);
CIRCA CE txtbuf_push(TxtBuf *tb, char c);
CIRCA CE txtbuf_pop(TxtBuf *tb, char *r);

/* Copy Operations */

CIRCA CE txtbuf_clear(TxtBuf *tb);
CIRCA CE txtbuf_cpy(TxtBuf *dst, TxtBuf *src);
CIRCA CE txtbuf_cpy_slice(TxtBuf *dst, TxtBuf *src, Slice s);
CIRCA CE txtbuf_cpy_cstr(TxtBuf *dst, char *src);
CIRCA CE txtbuf_cpy_cstr_slice(TxtBuf *dst, char *src, Slice s);
CIRCA CIRCA_VPRINTF(2) CE txtbuf_fmt_va(TxtBuf *tb, const char *fmt, va_list ap);
CIRCA CIRCA_VPRINTF(2) CE txtbuf_fmt(TxtBuf *tb, const char *fmt, ...);

/*
** Allocators
*/

CIRCA
TxtBuf txtbuf_init(void) {
  return (TxtBuf) {0};
}

CIRCA
CE txtbuf_alloc(TxtBuf *tb, size_t cap) {
  CE_CHECK(!tb, CE_NULL_ARG);
  CE_CHECK(!cap, CE_ZERO_ARG);
  tb->data = malloc(cap);
  CE_CRITICAL(!tb->data, CE_MALLOC);
  tb->data[0] = '\0';
  tb->cap = cap;
  return CE_OK;
}

CIRCA
CE txtbuf_realloc(TxtBuf *tb, size_t cap) {
  CE_CHECK(!tb, CE_NULL_ARG);
  CE_CHECK(!tb->data, CE_NULL_ARG);
  CE_CHECK(!cap, CE_ZERO_ARG);
  tb->data = realloc(tb->data, cap);
  CE_CRITICAL(!tb->data, CE_REALLOC);
  tb->cap = cap;
  if (cap <= tb->len) {
    tb->len = cap - 1;
    tb->data[tb->len] = '\0';
  }
  return CE_OK;
}

CIRCA
CE txtbuf_prealloc(TxtBuf *tb, size_t cap) {
  CE_CHECK(!tb, CE_NULL_ARG);
  CE_CHECK(!tb->data, CE_NULL_ARG);
  if (cap <= tb->cap)
    return CE_OK;
  return txtbuf_realloc(tb, cap);
}

CIRCA
CE txtbuf_shrink(TxtBuf *tb) {
  txtbuf_realloc(tb, tb->len + 1);
}

CIRCA
CE txtbuf_free(TxtBuf *tb) {
  CE_CHECK(!tb, CE_NULL_ARG);
  tb->cap = 0;
  tb->len = 0;
  free(tb->data);
  tb->data = NULL;
}

/*
** Accessors
*/

CIRCA
CE txtbuf_set(TxtBuf *tb, size_t a, char c) {
  CE_CHECK(!tb, CE_NULL_ARG);
  CE_CHECK(!tb->data, CE_NULL_ARG);
  CE_CHECK(a > tb->len, CE_OOB);

  if (a == tb->len) {
    if (c == '\0')
      return CE_OK;
    CE req_fail = txtbuf_prealloc(tb, a + 2);
    if (req_fail)
      return req_fail;
    tb->data[a + 1] = '\0';
    tb->len = a + 1;
  } else if (c == '\0') {
    tb->len = a;
  }
  
  tb->data[a] = c;
 
  return CE_OK;
}

CIRCA
CE txtbuf_get(TxtBuf *tb, size_t a, char *r) {
  CE_CHECK(!tb, CE_NULL_ARG);
  CE_CHECK(!tb->data, CE_NULL_ARG);
  CE_CHECK(a > tb->len, CE_OOB);
  *r = tb->data[a];
  return CE_OK;
}

CIRCA
CE txtbuf_push(TxtBuf *tb, char c) {
  return txtbuf_set(tb, tb->len, c);
}

CIRCA
CE txtbuf_pop(TxtBuf *tb, char *r) {
  CE_CHECK(!tb, CE_NULL_ARG);
  CE_CHECK(!tb->data, CE_NULL_ARG);
  char c;
  if (tb->len) {
    c = tb->data[--tb->len];
    tb->data[tb->len] = '\0';
  } else {
    c = '\0';
  }
  if (r)
    *r = c;
  return CE_OK;
}


/*
** Copy Operations
*/

CIRCA
CE txtbuf_clear(TxtBuf *tb) {
  CE_CHECK(!tb, CE_NULL_ARG);
  CE_CHECK(!tb->data, CE_NULL_ARG);
  tb->len = 0;
  tb->data[0] = '\0';
  return CE_OK;
}

CIRCA
CE txtbuf_cpy(TxtBuf *dst, TxtBuf *src) {
  CE_CHECK(!dst, CE_NULL_ARG);
  CE_CHECK(!dst->data, CE_NULL_ARG);
  CE_CHECK(!src, CE_NULL_ARG);
  CE_CHECK(!src->data, CE_NULL_ARG);
  size_t len = src->len;
  CE req_fail = txtbuf_prealloc(dst, len + 1);
  if (req_fail)
    return req_fail;
  memcpy(dst->data, src, len + 1);
  dst->len = len;
  return CE_OK;
}

CIRCA
CE txtbuf_cpy_slice(TxtBuf *dst, TxtBuf *src, Slice s) {
  CE_CHECK(!dst, CE_NULL_ARG);
  CE_CHECK(!dst->data, CE_NULL_ARG);
  CE_CHECK(!src, CE_NULL_ARG);
  CE_CHECK(!src->data, CE_NULL_ARG);
  CE_GUARD (1)
    if (!slice_contains(s, (Slice) {0, src->len}))
      return CE_OOB;

  size_t s_len = slice_len(s);

  CE req_fail = txtbuf_prealloc(dst, s_len + 1);
  if (req_fail)
    return req_fail;

  memcpy(dst->data, src->data + s.l, s_len);
  dst->len = s_len;
  dst->data[s_len] = '\0';

  return CE_OK;
}

CIRCA
CE txtbuf_cpy_cstr(TxtBuf *dst, char *src) {
  CE_CHECK(!dst, CE_NULL_ARG);
  CE_CHECK(!dst->data, CE_NULL_ARG);
  CE_CHECK(!src, CE_NULL_ARG);
  size_t len = strlen(src);
  CE req_fail = txtbuf_prealloc(dst, len + 1);
  if (req_fail)
    return req_fail;
  memcpy(dst->data, src, len + 1);
  dst->len = len;
  return CE_OK;
}

CIRCA
CE txtbuf_cpy_cstr_slice(TxtBuf *dst, char *src, Slice s) {
  CE_CHECK(!dst, CE_NULL_ARG);
  CE_CHECK(!dst->data, CE_NULL_ARG);
  CE_CHECK(!src, CE_NULL_ARG);
  CE_GUARD (1)
    if (!slice_contains(s, (Slice) {0, strlen(src)}))
      return CE_OOB;

  size_t s_len = slice_len(s);

  CE req_fail = txtbuf_prealloc(dst, s_len + 1);
  if (req_fail)
    return req_fail;

  memcpy(dst->data, src + s.l, s_len);
  dst->len = s_len;
  dst->data[s_len] = '\0';

  return CE_OK;
}

CIRCA CIRCA_VPRINTF(2)
CE txtbuf_fmt_va(TxtBuf *tb, const char *fmt, va_list ap) {
  CE_CHECK(!tb, CE_NULL_ARG);
  CE_CHECK(!tb->data, CE_NULL_ARG);
  CE_CHECK(!fmt, CE_NULL_ARG);
  va_list ap2;
  va_copy(ap2, ap);
  size_t len = vsnprintf(NULL, 0, fmt, ap2);
  va_end(ap2);
  CE req_fail = txtbuf_prealloc(tb, len + 1);
  if (req_fail)
    return req_fail;
  vsnprintf(tb->data, len, fmt, ap);
  tb->data[len] = '\0';
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
