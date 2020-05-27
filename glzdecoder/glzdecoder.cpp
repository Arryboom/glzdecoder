#include "image-decoder.hpp"
#include <stdio.h>
static uint32_t glz_id = 0;
unsigned char* readBmp(void)
{
	char file_str[200];
	uint32_t id = ++glz_id;
	sprintf(file_str, "d:\\tmp\\tmp\\%u.glz", id);
	FILE* fp;
	if ((fp = fopen(file_str, "rb")) == NULL)  //以二进制的方式打开文件
	{
		return FALSE;
	}
	if (fseek(fp, 0, 0))  //跳过BITMAPFILEHEADE
	{
		return FALSE;
	}
	UINT32 glz_size;
	fread(&glz_size, sizeof(UINT32), 1, fp);   //从fp中读取BITMAPINFOHEADER信息到infoHead中,同时fp的指针移动
	unsigned char* pBmpBuf = new unsigned char[glz_size];
	fread(pBmpBuf, sizeof(unsigned char), glz_size, fp);
	fclose(fp);   //关闭文件
	return pBmpBuf;
}

int main()
{	
	ImageDecoder* decoder = new ImageDecoder();
	while (1) {
		uint8_t* data = readBmp();
		decoder->glz_decode(data, NULL);
	}
	return 0;
}
