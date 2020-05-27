#include "glz-image.hpp"
class GlzDecoderWindow
{
public:
	GlzDecoderWindow();
	~GlzDecoderWindow();
	GlzImage** images;
	UINT32     nimages;
	UINT64     oldest;
	UINT64     tail_gap;
	void glz_decoder_window_add(GlzImage* image);
	void glz_decoder_window_release(uint64_t m_oldest);
	void* glz_decoder_window_bits(uint64_t id, uint32_t dist, uint32_t offset);
private:
	void glz_decoder_window_clear(void);
	void glz_decoder_window_resize(void);
};

