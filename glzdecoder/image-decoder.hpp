#include "glz-decoder.hpp"
class ImageDecoder
{
public:
	ImageDecoder();
	~ImageDecoder();
	void glz_decode(uint8_t* data, SpicePalette* palette);
private:
	GlzDecoder* glz_decoder;
	
};

