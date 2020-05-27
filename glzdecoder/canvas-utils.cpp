#include "canvas-utils.hpp"

typedef struct PixmanData {
	uint8_t* data;
	pixman_format_code_t format;
} PixmanData;

static void release_data(pixman_image_t* image, void* release_data)
{
	PixmanData* data = (PixmanData*)release_data;

	free(data->data);

	free(data);
}

static PixmanData* pixman_image_add_data(pixman_image_t* image)
{
	PixmanData* data;

	data = (PixmanData*)pixman_image_get_destroy_data(image);
	if (data == NULL) {
		data = (PixmanData*)calloc(1, sizeof(PixmanData));
		if (data == NULL) {
			//spice_error("out of memory");
		}
		pixman_image_set_destroy_function(image, release_data, data);
	}

	return data;
}

void spice_pixman_image_set_format(pixman_image_t* image, pixman_format_code_t format)
{
}

int spice_pixman_image_get_format(pixman_image_t* image, pixman_format_code_t* format)
{
	return 0;
}

pixman_image_t* surface_create(pixman_format_code_t format, int width, int height, int top_down)
{
	return nullptr;
}

pixman_image_t* surface_create_stride(pixman_format_code_t format, int width, int height, int stride)
{
	uint8_t* data;
	uint8_t* stride_data;
	pixman_image_t* surface;
	PixmanData* pixman_data;

	data = (uint8_t*)malloc(abs(stride) * height * sizeof(uint8_t));
	if (stride < 0) {
		stride_data = data + (-stride) * (height - 1);
	}
	else {
		stride_data = data;
	}

	surface = pixman_image_create_bits(format, width, height, (uint32_t*)stride_data, stride);

	if (surface == NULL) {
		free(data);
		data = NULL;
		//spice_error("create surface failed, out of memory");
	}

	pixman_data = pixman_image_add_data(surface);
	pixman_data->data = data;
	pixman_data->format = format;

	return surface;
}

pixman_image_t* alloc_lz_image_surface(pixman_format_code_t pixman_format, int width, int height, int gross_pixels, int top_down)
{
	int stride;
	pixman_image_t* surface = NULL;

	stride = (gross_pixels / height) * (PIXMAN_FORMAT_BPP(pixman_format) / 8);

	/* pixman requires strides to be 4-byte aligned */
	stride = SPICE_ALIGN(stride, 4);

	if (!top_down) {
		stride = -stride;
	}

	surface = surface_create_stride(pixman_format, width, height, stride);
	
	return surface;
}
