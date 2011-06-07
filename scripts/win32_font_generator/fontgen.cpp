#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

DWORD n1 = 0;
DWORD n2 = 0;

struct CHARACTER
{
	int CharCode;
	byte tex;
	int x;
	int y;
	int w;
	int aw;
	byte* data;
	int nData;
};

struct TGAINFOHEADER
{
	WORD w;
	WORD h;
	byte bpp, Flags;
};

short maxw = 512;
short bitmapw = 256;
int fonth = 32;
float scale = 1.0f;
char name[128] = {0};

byte *data;
bool ascii = 0;
long sx = 2;
long sy = 2;
long nCharacters = 0;
// windows context variables
BITMAPINFO* bi;
HDC dc;
HBITMAP hBMP;
HFONT hFont;
byte* bdata;

CHARACTER characters[65536];

void add(int ch)
{
	DWORD src;
	DWORD dst;

	// get character info
	ABC abc;
	WCHAR c[2] = {0};
	int w = 0;
	c[0] = ch;
	int cc = GetCharABCWidthsW(dc, ch, ch, &abc);
	w = abc.abcA + abc.abcB;
	// blit character onto DC
	memset(bdata, 255, bitmapw * bitmapw * 3);
	TextOutW(dc, 0, 0, c, 1);

	// fill character info
	CHARACTER* chr = &characters[nCharacters];
	chr->data = new byte[bitmapw * bitmapw];

	chr->CharCode = ch;
	w = chr->w = abc.abcA + abc.abcB;
	chr->aw = w + abc.abcC;
	chr->tex = 0;
	chr->aw < w ? chr->aw = w : w = chr->aw;
	
	// move to next row if we've reached the end
	if (sx + w > maxw)
	{
		sy += 4 + fonth;
		sx = 2;
	}
	
	chr->x = sx;
	chr->y = sy;
	chr->nData = 0;
	for (long y = sy; y <= sy + fonth; y++)
	{
		dst = y * maxw + sx;
		src = (bitmapw - 1 - y + sy) * bitmapw * 3;
		for (long x = sx; x < sx + w; x++)
		{
			data[dst] = chr->data[chr->nData] = 255 - (bdata[src] + bdata[src + 1] + bdata[src + 2]) / 3;
			dst++;
			chr->nData++;
			src += 3;
		}
	}
	
	// check for similar characters
	for (int i = 0; i < nCharacters; i++)
	{
		if (characters[i].nData == chr->nData)
		{
			if (memcmp(characters[i].data, chr->data, chr->nData) == 0)
			{
				n2++;
				if (characters[i].CharCode != 0)
				{
					printf("rejected %X, same as %X\n", ch, characters[i].CharCode);
					// this code if you want to keep the character but point it to 
					// an existing image
					*chr = characters[i];
					chr->nData = -1;
					chr->CharCode = ch;
					nCharacters++;
				}
				return;
			}
		}
	}
	sx += 4 + w;
	nCharacters++;
	n1++;
}	

long Power2(long size)
{
	long i = 1;
	while (i < size)
	{
		i *= 2;
	}
	return i;
}

void AddAll()
{
	int ranges[][2] = { // unicode ranges, according to wikipedia: http://en.wikipedia.org/wiki/Unicode
//		{0x0000, 0x001F}, // Basic Latin (non-printable)
		{0x0020, 0x007F}, // Basic Latin
/*
		{0x0080, 0x00FF}, // Latin-1 Supplement
		{0x0100, 0x01FF}, // Latin Extended A
		{0x0218, 0x021F}, // Latin Extended B
		{0x0370, 0x037F}, // Greek
		{0x0400, 0x04FF}, // Cyrillic
		{0x1E00, 0x1EFF}, // Latin Extended Additional
		{0x20A0, 0x20CF}, // Currency symbols
*/
/*
		{0x00A9, 0x00A9}, // Copyright
		{0x0106, 0x0107}, // C' c'
		{0x010C, 0x010D}, // Cv cv
		{0x0110, 0x0111}, // -D -d
		{0x0160, 0x0161}, // Sv sv
		{0x017D, 0x017E}, // Zv zv
*/
//		{0x,0x}, // 
//		{0x,0x}, // 
//		{0x,0x}, // 
//		{0x,0x}, // 
//		{0x,0x}, // 
//		{0x,0x}, // 
//		{0x,0x}, // 
//		{0x,0x}, // 
//		{0x,0x}, // 
//		{0x,0x}, // 
		{0, 0} // end marker
	};
	for (int j = 0; ranges[j][1] != 0; j++)
	{
		for (int i = ranges[j][0]; i <= ranges[j][1]; i++)
		{
			add(i);
			if (i % 100 == 0)
			{
				printf("%ld ( source_y=%i; n1=%ld; n2=%ld )\n", i, sy, n1, n2);
			}
		}
	}
	printf("-------------\ndone!\n\nSummary:\n\nCharacters written: %ld\nCharacters rejected %ld\n", n1, n2);
}

void Compile()
{
	if (sx > 1)
	{
		sy += 2 + 25;
		sx = 1;
	}

	int texw = maxw;
	int texh = Power2(sy + fonth);

	char filename[128] = "";
	sprintf(filename, "%s.tga", name);
	FILE* f = fopen(filename, "wb");

	byte Header[12] = {0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	TGAINFOHEADER iHeader;

	iHeader.bpp = 32;
	iHeader.w = texw;
	iHeader.h = texh;
	iHeader.Flags = 33;

	fwrite(Header, 1, 12, f);
	fwrite(&iHeader, 1, sizeof(iHeader), f);
	DWORD size = texw * texh;
	DWORD rowsize = texh, iEnd = 0xFFFFFFFF - rowsize + 1;
	for (DWORD i = 0; i < size; i++)
	{
		fputc(0xFF, f); fputc(0xFF, f); fputc(0xFF, f);
		fputc(data[i], f);
	}

	fclose(f);

	printf("done!\n\nSummary:\n\nCharacters written: %ld\nCharacters rejected %ld\n", n1, n2);
}

void WriteXML()
{
	char filename[128] = "";
	sprintf(filename, "%s.font", name);
	FILE* f = fopen(filename, "w");
	fprintf(f, "# -----------------------------------\n");
	fprintf(f, "# ATRES Font definition file\n");
	fprintf(f, "# -----------------------------------\n");
	fprintf(f, "Name=%s\n", name);
	fprintf(f, "Texture=%s.tga\n", name);
	fprintf(f, "Height=%d\n", fonth);
	fprintf(f, "Scale=%g\n", scale);
	fprintf(f, "# -----------------------------------\n");
	fprintf(f, "# Code|X|Y|Width|Height|Advance Width\n");
	fprintf(f, "-------------------------------------\n");
	CHARACTER* c;
	for (int i = 0; i < nCharacters; i++)
	{
		c = &characters[i];
		fprintf(f, "%d %d %d %d ", c->CharCode, c->x, c->y, c->w, c->aw);
		c->w == c->aw ? fprintf(f, "0\n") : fprintf(f, "%d\n", c->aw);
	}
	fclose(f);
}

int main(int argc, char** argv)
{
	if (argc != 3 && argc != 4)
	{
		printf("Wrong number of arguments supplied: FONTNAME FONTSIZE [SCALE]\n");
		return 1;
	}
	strcpy(name, argv[1]);
	sscanf(argv[2], "%d", &fonth);
	if (argc == 4)
	{
		sscanf(argv[3], "%f", &scale);
	}

	bool bold = 1;
	DWORD quality = PROOF_QUALITY;

	data = (byte*) malloc(maxw * (maxw * 4) * 2);
	memset(data, 0, maxw * (maxw * 4) * 2);

	bi = (BITMAPINFO*)malloc(sizeof(BITMAPINFO));
	memset(bi, 0, sizeof(BITMAPINFO));

	bi->bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
	bi->bmiHeader.biWidth       = bitmapw;
	bi->bmiHeader.biHeight      = bitmapw;
	bi->bmiHeader.biPlanes      = 1;
	bi->bmiHeader.biBitCount    = 24;
	bi->bmiHeader.biCompression = BI_RGB;

	dc = CreateCompatibleDC(NULL);

	hBMP = CreateDIBSection(dc, bi, DIB_RGB_COLORS, (void**)&bdata, NULL, 0);

	WCHAR fontname[128] = {0};
	mbstowcs((wchar_t*)fontname, name, strlen(name));
	hFont = CreateFontW(fonth, 0, 0, 0, 100 + bold * 300, 0, 0, 0, ANSI_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, quality, DEFAULT_PITCH, fontname);
	SelectObject(dc, hBMP);
	SelectObject(dc, hFont);

	AddAll();
	Compile();
	WriteXML();

	free(data);
	free(bi);
	DeleteObject(hBMP);
	DeleteObject(dc);
	return 0;
}