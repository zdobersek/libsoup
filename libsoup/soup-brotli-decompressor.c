#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "soup-brotli-decompressor.h"

#include <brotli/decode.h>
#include <gio/gio.h>

typedef struct {
	GObjectClass parent_class;

} SoupBrotliDecompressorClass;

struct _SoupBrotliDecompressor
{
	GObject parent_instance;

	BrotliDecoderState* brotli_decoder;
};

#define G_TYPE_SOUP_BROTLI_DECOMPRESSOR (soup_brotli_decompressor_get_type ())
#define SOUP_BROTLI_DECOMPRESSOR(o)		(G_TYPE_CHECK_INSTANCE_CAST ((o), G_TYPE_SOUP_BROTLI_DECOMPRESSOR, SoupBrotliDecompressor))

static void soup_brotli_decompressor_iface_init (GConverterIface *iface);

G_DEFINE_TYPE_WITH_CODE (SoupBrotliDecompressor, soup_brotli_decompressor, G_TYPE_OBJECT,
	G_IMPLEMENT_INTERFACE (G_TYPE_CONVERTER, soup_brotli_decompressor_iface_init))

SoupBrotliDecompressor *
soup_brotli_decompressor_new (void)
{
	SoupBrotliDecompressor *decompressor;

	decompressor = g_object_new (G_TYPE_SOUP_BROTLI_DECOMPRESSOR, NULL);

	return decompressor;
}

static void
soup_brotli_decompressor_init (SoupBrotliDecompressor *decompressor)
{
}

static void*
soup_brotli_decompressor_alloc (void* opaque, size_t size)
{
    return g_malloc (size);
}

static void
soup_brotli_decompressor_free (void* opaque, void* address)
{
    g_free (address);
}

static void
soup_brotli_decompressor_constructed (GObject *object)
{
	SoupBrotliDecompressor *decompressor;

    decompressor = SOUP_BROTLI_DECOMPRESSOR (object);

    decompressor->brotli_decoder = BrotliDecoderCreateInstance (
				soup_brotli_decompressor_alloc,
				soup_brotli_decompressor_free,
				NULL);
}

static void
soup_brotli_decompressor_finalize (GObject *object)
{
	SoupBrotliDecompressor *decompressor;

    decompressor = SOUP_BROTLI_DECOMPRESSOR (object);

    BrotliDecoderDestroyInstance (decompressor->brotli_decoder);
}

static void
soup_brotli_decompressor_class_init (SoupBrotliDecompressorClass *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

	gobject_class->constructed = soup_brotli_decompressor_constructed;
    gobject_class->finalize = soup_brotli_decompressor_finalize;
}

static GConverterResult
soup_brotli_decompressor_convert (GConverter *converter,
									const void *inbuf,
									gsize inbuf_size,
									void *outbuf,
									gsize outbuf_size,
									GConverterFlags flags,
									gsize *bytes_read,
									gsize *bytes_written,
									GError **error)
{
	SoupBrotliDecompressor *decompressor;
	BrotliDecoderResult res;
	gsize available_in, available_out;
    const uint8_t *next_in;
	uint8_t *next_out;

    decompressor = SOUP_BROTLI_DECOMPRESSOR (converter);

	if (decompressor->brotli_decoder == NULL)
		return G_CONVERTER_ERROR;

	available_in = inbuf_size;
	next_in = inbuf;
	available_out = outbuf_size;
	next_out = outbuf;

	res = BrotliDecoderDecompressStream (decompressor->brotli_decoder,
					&available_in, &next_in,
					&available_out, &next_out,
					NULL);

	if (res == BROTLI_DECODER_RESULT_ERROR) {
		g_set_error (error, G_IO_ERROR, G_IO_ERROR_FAILED, "");
		return G_CONVERTER_ERROR;
	}

	if (available_in > inbuf_size || available_out > outbuf_size)
		return G_CONVERTER_ERROR;

	*bytes_read = inbuf_size - available_in;
	*bytes_written = outbuf_size - available_out;

	if (res == BROTLI_DECODER_RESULT_SUCCESS)
        return G_CONVERTER_FINISHED;

	if (*bytes_read > 0 || *bytes_written > 0)
		return G_CONVERTER_CONVERTED;

	if (res == BROTLI_DECODER_RESULT_NEEDS_MORE_INPUT) {
        g_set_error_literal (error, G_IO_ERROR, G_IO_ERROR_PARTIAL_INPUT, "");
		return G_CONVERTER_ERROR;
	}

	if (res == BROTLI_DECODER_RESULT_NEEDS_MORE_OUTPUT) {
		g_set_error_literal (error, G_IO_ERROR, G_IO_ERROR_NO_SPACE, "");
        return G_CONVERTER_ERROR;
	}

	return G_CONVERTER_ERROR;
}

static void
soup_brotli_decompressor_reset (GConverter *converter)
{
	SoupBrotliDecompressor *decompressor;

    decompressor = SOUP_BROTLI_DECOMPRESSOR (converter);

    if (decompressor->brotli_decoder)
        BrotliDecoderDestroyInstance (decompressor->brotli_decoder);

    decompressor->brotli_decoder = BrotliDecoderCreateInstance (
				soup_brotli_decompressor_alloc,
				soup_brotli_decompressor_free,
				NULL);
}

static void
soup_brotli_decompressor_iface_init (GConverterIface *iface)
{
	iface->convert = soup_brotli_decompressor_convert;
	iface->reset = soup_brotli_decompressor_reset;
}
