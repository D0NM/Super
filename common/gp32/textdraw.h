#ifndef	__TEXTDRAW_H__
#define	__TEXTDRAW_H__

#define GPC_GT_BOTTOM		0x2
#define GPC_GT_HCENTER		0x8
#define GPC_GT_LEFT			0x10
#define GPC_GT_RIGHT		0x20
#define GPC_GT_TOP			0x40
#define GPC_GT_WORDBREAK	0x80		/* ÀÚµ¿ ÁÙ¹Ù²Þ */
#define GPC_GT_VCENTER		0x100

void GpTextDraw(GPDRAWSURFACE * ptgpds,	/* surface to draw text */
				GPRECT * cRect,		/* rectangle area to clipping text */
				unsigned int uFormat,	/* flag to specify the appearance of text */
				char * sour,	/* pointer to the text to draw */
				int nStart,		/* index of string to write from */
				int nCount,		/* number of characters to write */
				unsigned char color		/* color of text */);
#endif
