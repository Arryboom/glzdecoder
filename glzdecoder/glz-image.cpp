#include "glz-image.hpp"
GlzImage::GlzImage(struct glz_image_hdr* m_hdr, int type)
{
	if (!(type == LZ_IMAGE_TYPE_RGB32 || type == LZ_IMAGE_TYPE_RGBA)) {
		//return;
	}

	hdr = *m_hdr;
	surface = alloc_lz_image_surface
	(type == LZ_IMAGE_TYPE_RGBA ? PIXMAN_LE_a8r8g8b8 : PIXMAN_LE_x8r8g8b8,
		hdr.width, hdr.height, hdr.gross_pixels, hdr.top_down);
	pixman_image_ref(surface);
	data = (uint8_t*)pixman_image_get_data(surface);
	if (!hdr.top_down) {
		data = data - hdr.width * (hdr.height - 1) * 4;
	}
}

GlzImage::~GlzImage()
{	
	if (surface) {
		pixman_image_unref(surface);
	}
}