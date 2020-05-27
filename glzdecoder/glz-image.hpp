#include "canvas-utils.hpp"

struct glz_image_hdr {
	UINT64                id;
	LzImageType           type;
	UINT32                width;
	UINT32                height;
	UINT32                gross_pixels;
	bool                  top_down;
	UINT32                win_head_dist;
};

class GlzImage
{
public:
	GlzImage(struct glz_image_hdr* m_hdr, int type);
	~GlzImage();
	struct glz_image_hdr   hdr;
	pixman_image_t* surface;
	UINT8* data;
private:
	
};


