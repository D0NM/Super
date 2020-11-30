#ifndef __bfont_h__
#define __bfont_h__

//a single letter
struct BFLetter {
	//the offset for this letter's data
	//unsigned char *bin;
	//these values are for the char
	int width;
	int height;
	int x;
	int y;
};

//the whole font
struct BFont {
	//main allocated massive of fonts data
	unsigned char *source;
	struct BFLetter c[256];
	//main fonts colours
	unsigned int color[4];
	//transparent colour
	int transparent;
	//MAX values
	int width;
	int kor_w;	//ширина корейской буквы 8(
	int height;
	//compat...
	int chargap;
	int linegap;
	//the whole fonts matrix dimesions
	int mw;
	int mh;
	//font type 0-prop, 1-monosp, 3-system
	int	type;
};

int LoadFont(char *font_name,struct BFont *font, unsigned char*);
int FmlLoadFont(char *font_name,struct BFont *font, unsigned char*);
void Put1Chara(GPDRAWTAG * gptag, GPDRAWSURFACE *gpDraw, struct BFont *font, int x1, int y1, unsigned char i);
void Put1TrChara(GPDRAWTAG * gptag, GPDRAWSURFACE *gpDraw, struct BFont *font, int x1, int y1, unsigned char i);
void BCharOut(GPDRAWTAG * gptag, GPDRAWSURFACE *ptgpds ,struct BFont *font,int x,int y,char* sour);
void BTextOut(GPDRAWTAG * gptag, GPDRAWSURFACE *ptgpds ,struct BFont *font,int x,int y,char* sour);
int BTextWidthGet(struct BFont *font, const char * lpsz);
int BTextHeightGet(struct BFont *font, const char * lpsz);
int BTextLenGet(struct BFont *font, const char * str);
void BTextNOut(GPDRAWTAG * gptag, GPDRAWSURFACE * ptgpds,struct BFont *font, int x, int y, char * sour, int nStart, int nString);

#endif /*__bfont_h__*/