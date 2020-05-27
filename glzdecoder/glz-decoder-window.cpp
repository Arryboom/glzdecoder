#include "glz-decoder-window.hpp"
#include <stdio.h>

GlzDecoderWindow::GlzDecoderWindow()
{
	glz_decoder_window_clear();
}

GlzDecoderWindow::~GlzDecoderWindow()
{
}

void GlzDecoderWindow::glz_decoder_window_add(GlzImage* img)
{
	int slot = img->hdr.id % nimages;

	if (images[slot]) {
		/* need more space */
		glz_decoder_window_resize();
		slot = img->hdr.id % nimages;
	}

	images[slot] = img;

	/* close the gap */
	while (tail_gap <= img->hdr.id && images[tail_gap % nimages] != NULL) {
		tail_gap++;
	}
}

void GlzDecoderWindow::glz_decoder_window_release(uint64_t m_oldest)
{
	int slot;

	while (oldest < m_oldest) {
		slot = oldest % nimages;
		delete images[slot];
		oldest++;
	}
}

void* GlzDecoderWindow::glz_decoder_window_bits(uint64_t id, uint32_t dist, uint32_t offset)
{
	int slot = (id - dist) % nimages;

	if (images[slot] == NULL) {
		return NULL;
	}
	if (images[slot]->hdr.id != id - dist) {
		return NULL;
	}
	if (images[slot]->hdr.gross_pixels < offset) {
		return NULL;
	}

	return images[slot]->data + offset * 4;
}

void GlzDecoderWindow::glz_decoder_window_clear(void)
{	
	if (!(nimages == 0 || images != NULL)) {
		return;
	}

	for (int i = 0; i < nimages; i++) {
		if (images[i]) {
			delete images[i];
		}
	}

	nimages = 16;
	free(images);
	images = (GlzImage * *) malloc(sizeof(GlzImage*) *  nimages);
	memset(images, 0, sizeof(GlzImage*) * nimages);
	tail_gap = 0;
	oldest = 0;
}

void GlzDecoderWindow::glz_decoder_window_resize(void)
{
	int i, new_slot;

	printf("%s: array resize %u -> %u \n", __FUNCTION__, nimages, nimages * 2);
	GlzImage** new_images = (GlzImage * *) malloc(sizeof(void*) * nimages * 2);

	for (i = 0; i < nimages; i++) {
		if (images[i] == NULL) {
			/*
			 * We can have empty slots when images come in out of order, this
			 * can happen when a vm has multiple displays, since each display
			 * uses its own socket there is no guarantee that images
			 * originating from different displays are received in id order.
			 */
			continue;
		}
		new_slot = images[i]->hdr.id % (nimages * 2);
		new_images[new_slot] = images[i];
	}
	free(images);
	images = new_images;
	nimages *= 2;
}
