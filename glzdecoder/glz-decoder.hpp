#include "glz-decoder-window.hpp"
#include "drawable.hpp"

class GlzDecoder
{
public:
	GlzDecoder();
	~GlzDecoder();
	void decode(uint8_t* data, SpicePalette* palette);
private:
	UINT8* in_start;
	UINT8* in_now;
	GlzDecoderWindow* window;
	struct glz_image_hdr    image;
	void decode_header(void);
	uint32_t decode_32(void);
	uint64_t decode_64(void);
	size_t glz_rgb32_decode(GlzDecoderWindow* window,
		uint8_t* in_buf, uint8_t* out_buf, int size,
		uint64_t image_id, SpicePalette* plt);
};

