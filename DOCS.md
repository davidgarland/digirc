# circa_txtbuf

## Structs

### `TxtBuf`

```C
typedef struct {
  size_t cap;
  size_t len;
  char *data;
} TxtBuf;
```

The `TxtBuf` type is a dynamically resizable buffer for text that exists as a
wrapper over C `char*` with capacity and length fields attached, to enable
finding the length in `O(1)` time and avoid reallocations caused by constant
shrinking and growing. Functions should generally accept a `TxtBuf*` so as to
allow re-allocation of `data`.

As per the usual C stings, `len` does not include the null terminator; `cap`,
however, does. Therefore in a "full" string `len` will be one less than `cap`,
*NOT* equal to it.

## Allocators

### `txtbuf_init`

```C
TxtBuf txtbuf_init(void);
```

Returns a text buffer with `cap` set to `0`, `len` set to `0`, and `data` set
to `NULL`. When allocating a text buffer, the general pattern works out to be
something like:

```C
TxtBuf t = txtbuf_init();
txtbuf_alloc(&t, 1);
```

### `txtbuf_alloc`

```C
CE txtbuf_alloc(TxtBuf *tb, size_t cap);
```

Allocates a text buffer of the given capacity. It is assumed that it was
previously initialized with `txtbuf_init` or freed via `txtbuf_free`.

When `NDEBUG` is not defined:

- `CE_NULL_ARG` will be returned if `tb` is `NULL`.
- `CE_ZERO_ARG` will be returned if `cap` is `0`.

The following errors may always occur:

- `CE_MALLOC` will be returned if the internal call to `malloc` yields `NULL`.

### `txtbuf_realloc`

```C
CE txtbuf_realloc(TxtBuf *tb, size_t cap);
```

Re-allocates a text buffer to a given capacity. It is assumed that it was
previously allocated with `txtbuf_alloc`.

Note the following edge cases:

- If `cap <= tb->len`, then `tb->len` is set to `cap - 1` and the string is cut off.

When `NDEBUG` is not defined:

- `CE_NULL_ARG` will be returned if `tb` is `NULL`.
- `CE_NULL_ARG` will be returned if `tb->data` is `NULL`.
- `CE_ZERO_ARG` will be returned if `cap` is `0`.

The following errors may always occur:

- `CE_REALLOC` will be returned if the internal call to `realloc` yields `NULL`.

### `txtbuf_prealloc`

```C
CE txtbuf_prealloc(TxtBuf *tb, size_t cap);
```

Pre-allocates a buffer to have at least a given amount of memory. If the
buffer's capacity is greater than or equal to the one given, this is a no-op.

When `NDEBUG` is not defined:

- `CE_NULL_ARG` will be returned if `tb` is `NULL`.
- `CE_NULL_ARG` will be returned if `tb->data` is `NULL`.

### `txtbuf_shrink`

```C
CE txtbuf_shrink(TxtBuf *tb);
```

Shrink a buffer's capacity to be the same as its length plus one.

This function is defined as `txtbuf_realloc(tb, tb->len + 1)`. All the same
error cases apply.

### `txtbuf_free`

```C
CE txtbuf_free(TxtBuf *tb);
```

Free the memory associated with a buffer, then set its elements as if they had
been run through `txtbuf_init`. Double-free is not an issue, as running `free`
on `NULL` is safe.

When `NDEBUG` is not defined:

- `CE_NULL_ARG` will be returned if `tb` is `NULL`.

## Accessors

### txtbuf_set

```C
CE txtbuf_set(TxtBuf *tb, size_t a, char c);
```

In the buffer `tb`, set the character at the address `a` to be `c`.

Note the following edge cases:

- If `a == tb->len`, the character will be appended to the string and a new
null terminator will be added at `a + 1`.
- If `c == '\0'`, `tb->len` is set to `a`.

When `NDEBUG` is not defined:

- `CE_NULL_ARG` will be returned if `tb` is `NULL`.
- `CE_NULL_ARG` will be returned if `tb->data` is `NULL`.
- `CE_OOB` will be returned if `a` is greater than `tb->len`. (Writing at the null terminator is fine.)

The following errors may always occur in the case that `a == tb->len`:

- `CE_REALLOC` will be returned if the internal call to `txtbuf_prealloc` fails.

### txtbuf_get

```C
CE txtbuf_get(TxtBuf *tb, size_t a, char *r);
```

In the buffer `tb`, read the character at the address `a` into the variable
pointed at by `r`.

When `NDEBUG` is not defined:

- `CE_NULL_ARG` will be returned if `tb` is `NULL`.
- `CE_NULL_ARG` will be returned if `tb->data` is `NULL`.
- `CE_OOB` will be returned if `a` is greater than `tb->len`. (Reading the null terminator is fine.)

### txtbuf_push

```C
CE txtbuf_push(TxtBuf *tb, char c);
```

Append the character `c` to the buffer `tb`.

`txtbuf_push(tb, c)` is semantically equivalent to (and currently literally just
an alias for) `txtbuf_set(tb, tb->len, c)`; all the same error cases apply.

### txtbuf_pop

```C
CE txtbuf_pop(TxtBuf *tb, char *r);
```

Pops the last character of the buffer `tb` into the variable pointed at by `r`.

Note the following edge cases:

- If `tb->len == 0`, then the variable pointed at by `r` will be set to `'\0'` and the buffer's length will not be decreased.

When `NDEBUG` is not defined:

- `CE_NULL_ARG` will be returned if `tb` is `NULL`.
- `CE_NULL_ARG` will be returned if `tb->data` is `NULL`.'

## Copy Operations

### txtbuf_clear

```C
CE txtbuf_clear(TxtBuf *tb);
```

Clear the buffer `tb`. This sets `tb->len` to `0` and does not change the
capacity; if you need that, use `txtbuf_shrink` afterwards.

When `NDEBUG` is not defined:

- `CE_NULL_ARG` will be returned if `tb` is `NULL`.
- `CE_NULL_ARG` will be returned if `tb->data` is `NULL`.

### txtbuf_cpy

```C
CE txtbuf_cpy(TxtBuf *dst, TxtBuf *src);
```

Copy the contents of `src` into `dst`, reallocating `dst` as needed.

When `NDEBUG` is not defined:

- `CE_NULL_ARG` will be returned if `dst` is `NULL`.
- `CE_NULL_ARG` will be returned if `dst->data` is `NULL`.
- `CE_NULL_ARG` will be returned if `src` is `NULL`.
- `CE_NULL_ARG` will be returned if `src->data` is `NULL`.

The following errors may always occur:

- `CE_REALLOC` will be returned if the internal call to `txtbuf_prealloc` fails.

### txtbuf_cpy_slice

```C
CE txtbuf_cpy_slice(TxtBuf *dst, TxtBuf *src, Slice s);
```

Copy the slice specified by `s` of `src` into `dst`, reallocating `dst` as
needed.

When `NDEBUG` is not defined:

- `CE_NULL_ARG` will be returned if `dst` is `NULL`.
- `CE_NULL_ARG` will be returned if `dst->data` is `NULL`.
- `CE_NULL_ARG` will be returned if `src` is `NULL`.
- `CE_NULL_ARG` will be returned if `src->data` is `NULL`.
- `CE_OOB` will be returned if `s` is out of bounds. (`s` may not include the null terminator of `src`.)

The following errors may always occur:

- `CE_REALLOC` will be returned if the internal call to `txtbuf_prealloc` fails.

### txtbuf_cpy_cstr

```C
CE txtbuf_cpy_cstr(TxtBuf *dst, char *src);
```

Copy a null-terminated C string into a buffer.

When `NDEBUG` is not defined:

- `CE_NULL_ARG` will be returned if `dst` is `NULL`.
- `CE_NULL_ARG` will be returned if `dst->data` is `NULL`.
- `CE_NULL_ARG` will be returned if `src` is `NULL`.

The following errors may always occur:

- `CE_REALLOC` will be returned if the internal call to `txtbuf_prealloc` fails.

### txtbuf_cpy_cstr_slice

```C
CE txtbuf_cpy_cstr_slice(TxtBuf *dst, char *src, Slice s);
```

Copy the slice specified by `s` of a null-terminated C string `src` into `dst`,
reallocating `dst` as needed.

When `NDEBUG` is not defined:

- `CE_NULL_ARG` will be returned if `dst` is `NULL`.
- `CE_NULL_ARG` will be returned if `dst->data` is `NULL`.
- `CE_NULL_ARG` will be returned if `src` is `NULL`.
- `CE_OOB` will be returned if `s` is out of bounds. (`s` may not include the null terminator of `src`.)

The following errors may always occur:

- `CE_REALLOC` will be returned if the internal call to `txtbuf_prealloc` fails.

### txtbuf_fmt_va

```C
CE txtbuf_fmt_va(TxtBuf *tb, const char *fmt, va_list ap);
```

Format the buffer `tb` with the formatter `fmt` and variadic argument list `ap`.
This function is useful for implementing custom logging, but in general you
probably want `txtbuf_fmt` instead.

When `NDEBUG` is not defined:

- `CE_NULL_ARG` will be returned if `tb` is `NULL`.
- `CE_NULL_ARG` will be returned if `tb->data` is `NULL`.
- `CE_NULL_ARG` will be returned if `fmt` is `NULL`.

The following errors may always occur:

- `CE_REALLOC` will be returned if the internal call to `txtbuf_prealloc` fails.

### txtbuf_fmt

```C
CE txtbuf_fmt(TxtBuf *tb, const char *fmt, ...);
```

Format the buffer `tb` with the formatter `fmt` and the arguments `...`; this
works identically to `printf`.

The error cases are the same as that of `txtbuf_fmt_va`.
