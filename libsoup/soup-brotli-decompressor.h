#ifndef SOUP_BROTLI_DECOMPRESSOR_H
#define SOUP_BROTLI_DECOMPRESSOR_H 1

#include <gio/gio.h>

typedef struct _SoupBrotliDecompressor SoupBrotliDecompressor;

GType soup_brotli_decompressor_get_type (void) G_GNUC_CONST;

SoupBrotliDecompressor *soup_brotli_decompressor_new (void);

#endif /* SOUP_BROTLI_DECOMPRESSOR_H */
