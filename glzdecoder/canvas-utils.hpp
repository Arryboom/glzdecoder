#include "pixman-utils.hpp"


void spice_pixman_image_set_format(pixman_image_t* image,
	pixman_format_code_t format);
int spice_pixman_image_get_format(pixman_image_t* image, pixman_format_code_t* format);

pixman_image_t* surface_create(pixman_format_code_t format, int width, int height, int top_down);

pixman_image_t* surface_create_stride(pixman_format_code_t format, int width, int height, int stride);

pixman_image_t* alloc_lz_image_surface(pixman_format_code_t pixman_format, int width,
	int height, int gross_pixels, int top_down);

