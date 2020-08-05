#include "image-decoder.hpp"
#include <stdio.h>
#include <time.h>

static uint32_t glz_id = 0;
unsigned char* readBmp(void)
{
	char file_str[200];
	uint32_t id = ++glz_id;
    sprintf(file_str, "d:\\tmp\\%u.glz", id);
    //sprintf(file_str, "%u.glz", id);
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

static inline uint64_t get_time()
{
    time_t clock;
    timeval now;
    struct tm tm;
    SYSTEMTIME wtm;
    GetLocalTime(&wtm);
    tm.tm_year = wtm.wYear - 1900;
    tm.tm_mon = wtm.wMonth - 1;
    tm.tm_mday = wtm.wDay;
    tm.tm_hour = wtm.wHour;
    tm.tm_min = wtm.wMinute;
    tm.tm_sec = wtm.wSecond;
    tm.tm_isdst = -1;
    clock = mktime(&tm);
    now.tv_sec = clock;
    now.tv_usec = wtm.wMilliseconds * 1000;
    return (uint64_t)now.tv_sec * 1000000 + (uint64_t)now.tv_usec;
}

int main()
{	
	ImageDecoder* decoder = new ImageDecoder();
	while (1) {
		uint8_t* data = readBmp();
        uint64_t start = get_time();
		decoder->glz_decode(data, NULL);
        uint64_t time = get_time() - start;
        printf("decode time once is %u ms\n", time / 1000);
	}
	return 0;
}
