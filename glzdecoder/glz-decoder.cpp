#include "glz-decoder.hpp"
#include <cassert>
#include <stdio.h>
GlzDecoder::GlzDecoder()
{
	window = new GlzDecoderWindow();
}

GlzDecoder::~GlzDecoder()
{
	delete window;
}

static void saveBmpFile(unsigned char* pImgData, int width, int height, int imgLength)
{	
	static int i = 0;
	char fileName[50] = { 0 };
	sprintf_s(fileName, "d:\\tmp\\tmp\\%d.bmp", ++i);

	BITMAPFILEHEADER bmheader;
	memset(&bmheader, 0, sizeof(bmheader));
	bmheader.bfType = 0x4d42;     //图像格式。必须为'BM'格式。  
	bmheader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER); //从文件开头到数据的偏移量  
	bmheader.bfSize = imgLength + bmheader.bfOffBits;//文件大小  

	BITMAPINFOHEADER bmInfo;
	memset(&bmInfo, 0, sizeof(bmInfo));
	bmInfo.biSize = sizeof(bmInfo);
	bmInfo.biWidth = width;
	bmInfo.biHeight = height;
	bmInfo.biPlanes = 1;
	bmInfo.biBitCount = 32;
	bmInfo.biCompression = BI_RGB;

	HANDLE hFile = CreateFileA(fileName, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
	if (hFile != INVALID_HANDLE_VALUE) {
		DWORD dwWritten;
		BOOL bRet = WriteFile(hFile, &bmheader, sizeof(BITMAPFILEHEADER), &dwWritten, NULL);
		assert(TRUE == bRet);
		bRet = WriteFile(hFile, &bmInfo, sizeof(BITMAPINFOHEADER), &dwWritten, NULL);
		assert(TRUE == bRet);
		bRet = WriteFile(hFile, pImgData, imgLength, &dwWritten, NULL);
		assert(TRUE == bRet);
		CloseHandle(hFile);
	}
}

static uint8_t* surface_to_pixel(pixman_image_t* surface)
{
	static uint32_t file_id = 0;
	int i, j;
	char file_str[200];
	int depth = pixman_image_get_depth(surface);

	if (depth != 24 && depth != 32) {
		return NULL;
	}

	uint8_t* data = (uint8_t*)pixman_image_get_data(surface);
	int width = pixman_image_get_width(surface);
	int height = pixman_image_get_height(surface);
	int stride = pixman_image_get_stride(surface);
	uint8_t* output = new uint8_t[abs(stride) * height];
	uint8_t* dest = output;
	for (i = 0; i < height; i++, data += stride) {
		uint8_t* now = data;
		for (j = 0; j < width; j++) {
			memcpy(dest, now, 4);
			dest += 4;
			now += 4;
		}
	}
	return output;
	
}
void GlzDecoder::decode(uint8_t* data, SpicePalette* palette)
{
	LzImageType decoded_type;
	size_t n_in_bytes_decoded;

	in_start = data;
	in_now = data;

	decode_header();

	if (image.type == LZ_IMAGE_TYPE_RGBA) {
		decoded_type = LZ_IMAGE_TYPE_RGBA;
	}
	else {
		decoded_type = LZ_IMAGE_TYPE_RGB32;
	}

	GlzImage* decoded_image = new GlzImage(&image, decoded_type);

	n_in_bytes_decoded = glz_rgb32_decode(window, in_now, decoded_image->data,
		image.gross_pixels, image.id, palette);
	
	
	saveBmpFile(surface_to_pixel(decoded_image->surface), image.width, image.height, image.gross_pixels * 4);

	in_now += n_in_bytes_decoded;

	/*if (image.type == LZ_IMAGE_TYPE_RGBA) {
		glz_rgb_alpha_decode(window, in_now, decoded_image->data,
			image.gross_pixels, image.id, palette);
	}*/

	window->glz_decoder_window_add(decoded_image);

	{ /* release old images from last tail_gap, only if the gap is closed  */
		uint64_t oldest;
		GlzImage* image = window->images[(window->tail_gap - 1) % window->nimages];

		if (image == NULL) {
			return;
		}

		oldest = image->hdr.id - image->hdr.win_head_dist;
		window->glz_decoder_window_release(oldest);
	}
}

void GlzDecoder::decode_header(void)
{
	uint32_t magic;
	uint32_t version;
	uint32_t stride;
	uint8_t tmp;

	magic = decode_32();
	if (magic != LZ_MAGIC) {
		return;
	}

	version = decode_32();
	if (version != LZ_VERSION) {
		return;
	}

	tmp = *(in_now++);

	image.type = (LzImageType)(tmp & LZ_IMAGE_TYPE_MASK);
	image.top_down = (tmp >> LZ_IMAGE_TYPE_LOG) ? true : false;
	image.width = decode_32();
	image.height = decode_32();
	stride = decode_32();

	if (IS_IMAGE_TYPE_PLT[image.type]) {
		image.gross_pixels = stride * PLT_PIXELS_PER_BYTE[image.type] * image.height;
	}
	else {
		image.gross_pixels = image.width * image.height;
	}

	image.id = decode_64();
	image.win_head_dist = decode_32();

	/*
	printf("%s: %ux%u, id %" PRIu64 ", ref %" PRIu64,
			__FUNCTION__,
			d->image.width, d->image.height, d->image.id,
			d->image.id - d->image.win_head_dist);
	*/
}

uint32_t GlzDecoder::decode_32(void)
{
	uint32_t word = 0;
	word |= *(in_now++);
	word <<= 8;
	word |= *(in_now++);
	word <<= 8;
	word |= *(in_now++);
	word <<= 8;
	word |= *(in_now++);
	return word;
}

uint64_t GlzDecoder::decode_64(void)
{
	uint64_t long_word = decode_32();
	long_word <<= 32;
	long_word |= decode_32();
	return long_word;
}

#define LZ_RGB32
typedef struct rgb32_pixel_t {
	uint8_t b;
	uint8_t g;
	uint8_t r;
	uint8_t pad;
} rgb32_pixel_t;
#define OUT_PIXEL rgb32_pixel_t
#define COPY_COMP_PIXEL(in, out) {  \
    out->b = *(in++);               \
    out->g = *(in++);               \
    out->r = *(in++);               \
    out->pad = 0;                   \
    out++;                          \
}
#define COPY_PIXEL(p, out) (*(out++) = p)
#define COPY_REF_PIXEL(ref, out) (*(out++) = *(ref++))

#define LZ_EXPECT_CONDITIONAL(c) (c)
#define LZ_UNEXPECT_CONDITIONAL(c) (c)

size_t GlzDecoder::glz_rgb32_decode(GlzDecoderWindow* window, uint8_t* in_buf, uint8_t* out_buf, int size, uint64_t image_id, SpicePalette* plt)
{
	uint8_t* ip = in_buf;
	OUT_PIXEL* out_pix_buf = SPICE_ALIGNED_CAST(OUT_PIXEL*, out_buf);
	OUT_PIXEL* op = out_pix_buf;
	OUT_PIXEL* op_limit = out_pix_buf + size;

	uint32_t ctrl = *(ip++);
	int loop = true;

	do {
		if (ctrl >= MAX_COPY) { // reference (dictionary/RLE)
			OUT_PIXEL* ref = op;
			uint32_t len = ctrl >> 5;
			uint8_t pixel_flag = (ctrl >> 4) & 0x01;
			uint32_t pixel_ofs = (ctrl & 0x0f);
			uint8_t image_flag;
			uint32_t image_dist;

			/* retrieving the referenced images, the offset of the first pixel,
			   and the match length */

			uint8_t code;
			//len--; // TODO: why do we do this?

			if (len == 7) { // match length is bigger than 7
				do {
					code = *(ip++);
					len += code;
				} while (code == 255); // remaining of len
			}
			code = *(ip++);
			pixel_ofs += (code << 4);

			code = *(ip++);
			image_flag = (code >> 6) & 0x03;
			if (!pixel_flag) { // short pixel offset
				int i;
				image_dist = code & 0x3f;
				for (i = 0; i < image_flag; i++) {
					code = *(ip++);
					image_dist += (code << (6 + (8 * i)));
				}
			}
			else {
				int i;
				pixel_flag = (code >> 5) & 0x01;
				pixel_ofs += (code & 0x1f) << 12;
				image_dist = 0;
				for (i = 0; i < image_flag; i++) {
					code = *(ip++);
					image_dist += (code << 8 * i);
				}


				if (pixel_flag) { // very long pixel offset
					code = *(ip++);
					pixel_ofs += code << 17;
				}
			}

#if defined(LZ_PLT) || defined(LZ_RGB_ALPHA)
			len += 2; // length is biased by 2 (fixing bias)
#elif defined(LZ_RGB16)
			len += 1; // length is biased by 1  (fixing bias)
#endif
			if (!image_dist) {
				pixel_ofs += 1; // offset is biased by 1 (fixing bias)
			}

#if defined(TO_RGB32)
#if defined(PLT4_BE) || defined(PLT4_LE) || defined(PLT1_BE) || defined(PLT1_LE)
			pixel_ofs = CAST_PLT_DISTANCE(pixel_ofs);
			len = CAST_PLT_DISTANCE(len);
#endif
#endif

			if (!image_dist) { // reference is inside the same image
				ref -= pixel_ofs;
				if (!(ref + len <= op_limit)) {
					return 0;
				}
				if (!(ref >= out_pix_buf)) {
					return 0;
				}
			}
			else {
				ref = (OUT_PIXEL * )window->glz_decoder_window_bits(image_id, image_dist, pixel_ofs);
			}

			if (!(ref != NULL)) {
				return 0;
			}
			if (!(op + len <= op_limit)) {
				return 0;
			}

			/* copying the match*/

			if (ref == (op - 1)) { // run (this will never be called in PLT4/1_TO_RGB because the
								  // number of pixel copied is larger then one...
				/* optimize copy for a run */
				OUT_PIXEL b = *ref;
				for (; len; --len) {
					COPY_PIXEL(b, op);
					if (!(op <= op_limit)) {
						return 0;
					}
				}
			}
			else {
				for (; len; --len) {
					COPY_REF_PIXEL(ref, op);
					if (!(op <= op_limit)) {
						return 0;
					}
				}
			}
		}
		else { // copy
			ctrl++; // copy count is biased by 1
#if defined(TO_RGB32) && (defined(PLT4_BE) || defined(PLT4_LE) || defined(PLT1_BE) || \
                                                                                   defined(PLT1_LE))
			g_return_val_if_fail(op + CAST_PLT_DISTANCE(ctrl) <= op_limit, 0);
#else
			if (!(op + ctrl <= op_limit)) {
				return 0;
			}
#endif

#if defined(TO_RGB32) && defined(LZ_PLT)
			g_return_val_if_fail(plt, 0);
			COPY_COMP_PIXEL(ip, op, plt);
#else
			COPY_COMP_PIXEL(ip, op);
#endif
			if (!(op <= op_limit)) {
				return 0;
			}

			for (--ctrl; ctrl; ctrl--) {
#if defined(TO_RGB32) && defined(LZ_PLT)
				g_return_val_if_fail(plt, 0);
				COPY_COMP_PIXEL(ip, op, plt);
#else
				COPY_COMP_PIXEL(ip, op);
#endif
				if (!(op <= op_limit)) {
					return 0;
				}
			}
		} // END REF/COPY

		if (LZ_EXPECT_CONDITIONAL(op < op_limit)) {
			ctrl = *(ip++);
		}
		else {
			loop = false;
		}
	} while (LZ_EXPECT_CONDITIONAL(loop));

	return (ip - in_buf);
}
