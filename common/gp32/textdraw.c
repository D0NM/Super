#include "gpdef.h"
#include "gpstdlib.h"
#include "gpgraphic.h"
#include "gpfont.h"
#include "textdraw.h"

typedef struct line_info	/* structure for line information */
{
	char *line;	/* data of this line (in ASCII code) */
	int	width;		/* width of the line (in pixels) */
} line_info;


#if	1
line_info *realloc_array
(
	line_info *p,	/* pointer to old memory */
	int nline,	/* new number of lines */
	int *width	/* pointer to line width of current line */
)
{
	line_info *p_temp = (line_info *)gp_mem_func.malloc(nline * sizeof(line_info));	/* allocate new memory */
	int	i;
	BGFONTINFO	font_info;
//	int			font_h;

	GpSysFontGet(&font_info);
//	font_h = (font_info.eng_h > font_info.kor_h ? font_info.eng_h : font_info.kor_h);

	for(i = 0 ; i < nline - 1 ; i++)	/* for each line, copy old data */
		p_temp[i].width = p[i].width;
	p_temp[nline - 1].width = *width - font_info.chargap;	/* in fact, actual line width is narrower */
	*width = 0;				/* reset line width for next line */
	if(p)	gp_mem_func.free((void *)p);	/* free old memory */
	return p_temp;				/* return new memory pointer */
}
#endif

void GpTextDraw(GPDRAWSURFACE * ptgpds,	/* surface to draw text */
				GPRECT * cRect,		/* rectangle area to clipping text */
				unsigned int uFormat,	/* flag to specify the appearance of text */
				char * sour,	/* pointer to the text to draw */
				int nStart,		/* index of string to write from */
				int nCount,		/* number of characters to write */
				unsigned char color		/* color of text */)
{
	int	nline = 1;		/* number of lines */
	int	x, y;			/* coordinate of text to be drawn */
	char *p_sub_start;	/* buffer to store after extracting sub-string */
	char *p_buf_start;	/* buffer to store after handling CRr/LF */
	char *p_sub;		/* pointer to p_sub_start */
	char *p_buf;		/* pointer to p_buf_start */
	line_info *p_sub_array = 0;	/* array of lines of text */
	int	i;
	GPDRAWTAG _font_gdtag;
	BGFONTINFO	font_info;
	int			font_h;

	GpSysFontGet(&font_info);
	font_h = (font_info.eng_h > font_info.kor_h ? font_info.eng_h : font_info.kor_h);

#ifdef __GP_WIN_LAYER
	if ( !ptgpds || !sour ) return;
#endif

	_font_gdtag.clip_x = 0;
	_font_gdtag.clip_y = 0;
	_font_gdtag.clip_w = ptgpds->buf_w;
	_font_gdtag.clip_h = ptgpds->buf_h;

	p_sub = p_sub_start = (char *)gp_mem_func.malloc(nCount + 1);		/* memory allocation */
	p_buf = p_buf_start = (char *)gp_mem_func.malloc(nCount * 2 + 1);	/* memory allocation (double size for worst case) */
/****************************************************************************
 *                                                                          *
 *         extract sub string (into p_sub_start)                            *
 *                                                                          *
 ****************************************************************************/
	gp_str_func.strncpy((char *)p_sub_start, (const char *)(sour + nStart), nCount);	/* extract sub-string */
	p_sub_start[nCount] = '\0';	/* pad NULL to make string valid */
/****************************************************************************
 *                                                                          *
 *         replace cr/lf caracter (into p_buf_start)                        *
 *         handle GT_WORDBREAK flag                                         *
 *                                                                          *
 ****************************************************************************/
 	{	/* dummy block to make some variables local */
		int	flag_cr = 0;		/* flag indicating if CR appeared */
		int	flag_kor = 0;		/* flag indicating if previous character is the first part of korean */
		int	width = 0;		/* width of this line (in pixels) */
		while(*p_sub)	/* for the whole string */
		{
			if((*p_sub == '\r') || (*p_sub == '\n'))	/* if CR or LF appeared */
			{
				if(flag_cr && (*p_sub == '\n'))		/* if previous character was CR and current character is LF, */
				{
					p_sub++;			/* skip this character. (i.e, CRLF is replaced by LF) */
					continue;			/* and examine next character */
				}
				if(*p_sub == '\r')	flag_cr = 1;	/* if CR appeared, set CR flag */
				else			flag_cr = 0;	/* else clear CR flag */
				*(p_buf++) = '\n';			/* put LF (CR is replaced by LF) */
				p_sub_array = realloc_array(p_sub_array, nline++, &width);	/* reallocate array memory */
				flag_kor = 0;				/* clear flag */
				p_sub++;				/* examine next character */
			}
			else						/* in case of normal(?) character */
			{
				int	flag_kor_temp = flag_kor;	/* temporary flag to use in case GT_WORDBREAK */
				if(*p_sub & 0x80)			/* if this character is korean */
				{
					if(flag_kor)			/* if this character is second part of korean */
						width += font_info.chargap + font_info.kor_w;
					flag_kor = (flag_kor + 1) & 0x01;			/* inverse flag */
				}
				else					/* if this character is normal(english, number, etc) */
				{
					flag_kor = 0;
					width += font_info.eng_w + font_info.chargap;
				}
				if((uFormat & 	GPC_GT_WORDBREAK) && (width > (cRect->right - cRect->left)))/* if WORDBREAK flag is set and this line is too wide */
				{
					if(*p_sub & 0x80)		/* if this character is korean */
					{
						if(flag_kor_temp)	/* if this character is second part of korean */
						{
							width -= (font_info.chargap + font_info.kor_w);	/* decrease width by 1 korean character */
							p_sub--;		/* next time, examine previous character again */
							*(p_buf - 1) = '\n';	/* overwrite previous character (first part of korean) by LF */
						}
						else			/* if this character is first part of korean */
						{
							*(p_buf++) = '\n';	/* put LF by force */
							/* don't increase p_sub : next time, examine this character again */
						}
					}
					else	/* if this character is normal(english, number, etc) */
					{
						width -= (font_info.chargap + font_info.eng_w);
						*(p_buf++) = '\n';	/* put LF by force */
						/* don't increase p_sub : next time, examine this character again */
					}
					flag_kor = 0;			/* clear flag */
					p_sub_array = realloc_array(p_sub_array, nline++, &width);	/* reallocate array memory */
				}
				else
				{
					*(p_buf++) = *p_sub;			/* put current character in buffer */
					flag_cr = 0;				/* clear CR flag */
					p_sub++;					/* examine next character */
				}
			}
		}
		*p_buf = '\0';						/* pad NULL to make string valid */
		p_sub_array = realloc_array(p_sub_array, nline, &width);	/* reallocate array memory */
	}
/****************************************************************************
 *                                                                          *
 *         split into several lines (into p_sub_array)                      *
 *                                                                          *
 ****************************************************************************/
	p_buf = p_buf_start;		/* initialize the pointer to point to first character */
	for(i = 0 ; i < nline ; i++)	/* for each line */
	{
		char *p_start;	/* pointer to first character of this line */
		int	len;		/* length of this line */
		p_start = p_buf;	/* initialize the pointer to point to first character of this line */
		len = 0;		/* initialize the length of line */
		while((*(p_buf) != '\n') && (*(p_buf) != '\0'))	/* until the end of this line, for each character... */
		{
			p_buf++;	/* examine next character */
			len++;		/* increase the length of this line */
		}
		p_buf++;		/* skip the character indicating the end of this line ('\n' or '\0') */
		p_sub_array[i].line = (char *)gp_mem_func.malloc(len + 1);	/* allocate memory for this line */
		gp_str_func.strncpy((char *)p_sub_array[i].line, (const char *)p_start, len);	/* copy this line */
		p_sub_array[i].line[len] = '\0';	/* pad NULL */
	}
/****************************************************************************
 *                                                                          *
 *        calculate y-coordinate                                            *
 *                                                                          *
 ****************************************************************************/
 	{	/* dummy block to make some variables local */
 		int text_h = (font_h + font_info.linegap) * nline -  font_info.linegap;	/* calculate the height of the text */
	 	
	 	if(uFormat & GPC_GT_VCENTER)	/* vertically-centered-aligned */
	 		y = cRect->top + ((cRect->bottom - cRect->top) - text_h)/2;
	 	else if(uFormat & GPC_GT_BOTTOM)	/* bottom-aligned */
	 		y = cRect->top + cRect->bottom - cRect->top - text_h;
	 	else	/* (uFormat & GPC_GT_TOP)	 top-aligned */
	 		y = cRect->top;	
	 }
/****************************************************************************
 *                                                                          *
 *        calculate x-coordinate & draw text                                *
 *                                                                          *
 ****************************************************************************/
	for(i = 0 ; i < nline ; i++)
	{
		if(uFormat & GPC_GT_HCENTER)	/* horizontally-centered-aligned */
			x = cRect->left + (cRect->right - cRect->left - p_sub_array[i].width)/2;
		else if(uFormat & GPC_GT_RIGHT)	/* right-aligned */
			x = cRect->left + cRect->right - cRect->left - p_sub_array[i].width;
		else /* uFormat & GPC_GT_LEFT)	 left-aligned */
			x = cRect->left;
		GpTextOut(&_font_gdtag,
			ptgpds,
			x,
			y + (font_h + font_info.linegap)*i,
			p_sub_array[i].line,
			color);	/* draw this line !!! */
	}
/****************************************************************************
 *                                                                          *
 *        free memory                                                       *
 *                                                                          *
 ****************************************************************************/
	gp_mem_func.free((void *)p_sub_start);	
	gp_mem_func.free((void *)p_buf_start);
	for(i = 0 ; i < nline ; i++)	/* free the memory for each line (NEVER do this operation after freeing p_sub_array!!!!) */
		gp_mem_func.free((void *)(p_sub_array[i].line));
	gp_mem_func.free((void *)p_sub_array);
}
