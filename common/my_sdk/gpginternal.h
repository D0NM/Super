#ifndef	__GPGINTERNAL_H__
#define	__GPGINTERNAL_H__

#define	_GPPHYSICAL_W			240
#define	_GPPHYSICAL_H			320


typedef struct tagGPCLIP{
	int dx;
	int dy;
	int width;
	int height;
	int sx;
	int sy;
	int sw;
	int sh;
	int clip_l;
	int clip_t;
	int clip_r;
	int clip_b;
	int buf_h;
	int flagflip;
	int flag;
}_GPCLIP;

typedef struct tagGPDRAWINFO
{
	unsigned char * dest_addr;
	int dx;
	int dy;
	int width;
	int height;
	unsigned char * sour_addr;
	int sx;
	int sy;
	int pitch;
	int tgpitch;
	int color;
}_GPDRAWINFO;

extern _GpClipCheck(_GPCLIP *);
extern BgMakerBitBlt(_GPDRAWINFO *);
extern BgMakerTransBlt(_GPDRAWINFO *);
extern BgMakerLRBlt(_GPDRAWINFO *);
extern BgMakerTransLRBlt(_GPDRAWINFO *);
extern BgMakerUDBlt(_GPDRAWINFO *);
extern BgMakerTransUDBlt(_GPDRAWINFO *);
extern BgMakerFillColor(_GPDRAWINFO *);

extern BgMakerEllipse(GPDRAWSURFACE *,int,int,int,int,unsigned char);

extern _GpClipCheck16(_GPCLIP *);
extern BgMakerBitBlt16(_GPDRAWINFO *);
extern BgMakerTransBlt16(_GPDRAWINFO *);
extern BgMakerLRBlt16(_GPDRAWINFO *);
extern BgMakerTransLRBlt16(_GPDRAWINFO *);
extern BgMakerUDBlt16(_GPDRAWINFO *);
extern BgMakerTransUDBlt16(_GPDRAWINFO *);
extern BgMakerFillColor16(_GPDRAWINFO *);

#endif	/* __GPGINTERNAL_H__ */