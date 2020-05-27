#include "image-decoder.hpp"
ImageDecoder::ImageDecoder()
{
	glz_decoder = new GlzDecoder();
}

ImageDecoder::~ImageDecoder()
{
	delete glz_decoder;
}

void ImageDecoder::glz_decode(uint8_t* data, SpicePalette* palette)
{
	glz_decoder->decode(data, palette);
}