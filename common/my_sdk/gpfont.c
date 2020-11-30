#include "gpstdlib.h"
#include "gpfont.h"
#include "gpfinternal.h"
#include "korfont.dat"

BGFONTINFO _g_font;
int _g_font_h;
int _korfont_size;
int _engfont_size;
 	
extern const unsigned char fontresKor[];
extern const unsigned char fontresEng[];

unsigned char * _pt_font_kor;
unsigned char * _pt_font_eng;

void GpFontInit(BGFONTINFO * ptr)
{
	_g_font.kor_w = ptr->kor_w;
	_g_font.kor_h = ptr->kor_h;
	_g_font.eng_w = ptr->eng_w;
	_g_font.eng_h = ptr->eng_h;
	if (_g_font.eng_w > _g_font.kor_w)
		_g_font.chargap = (_g_font.eng_w * ptr->chargap)/100;
	else
		_g_font.chargap = (_g_font.kor_w * ptr->chargap)/100;
	_g_font_h = (_g_font.eng_h > _g_font.kor_h ? _g_font.eng_h : _g_font.kor_h);
	_g_font.linegap = (_g_font_h * ptr->linegap)/100;
	_korfont_size = ((_g_font.kor_h + 7) >> 3)*_g_font.kor_w;
	_engfont_size = ((_g_font.eng_h + 7) >> 3)*_g_font.eng_w;
}

void GpFontResSet(unsigned char *p_kor, unsigned char *p_eng)
{
	_pt_font_kor = p_kor;
	_pt_font_eng = p_eng;
}

unsigned char * GpKorFontResGet(void)
{
	return (_pt_font_kor);
}

unsigned char * GpEngFontResGet(void)
{
	return (_pt_font_eng);
}

void GpSysFontGet(BGFONTINFO *pFinfo)
{
	pFinfo->kor_w = _g_font.kor_w;
	pFinfo->kor_h = _g_font.kor_h;
	pFinfo->eng_w = _g_font.eng_w;
	pFinfo->eng_h = _g_font.eng_h;
	pFinfo->chargap = _g_font.chargap;
	pFinfo->linegap = _g_font.linegap;
}

int GpTextWidthGet(const char * lpsz)
{
	int result = 0, cur_result = _g_font.chargap;
	
	while(*lpsz != 0)
	{
		if((*lpsz) & 0x80) //korean
		{
			lpsz += 2;
			cur_result += _g_font.kor_w + _g_font.chargap;
		}
		else if ((*lpsz) == '\r' || (*lpsz) == '\n')
		{
			lpsz++;
			if (cur_result > result)
			{
				result = cur_result;
				cur_result = _g_font.chargap;
			}
			if ((*lpsz) == '\r' || (*lpsz) == '\n')
				lpsz++;
		}
		else
		{
			lpsz++;
			cur_result += _g_font.eng_w + _g_font.chargap;
		}	
	}
	if (cur_result > result)
		result = cur_result;
	if (result <= _g_font.chargap)
		result = 0;
	return result;
}

int GpTextHeightGet(const char * lpsz)
{
	int result = _g_font_h;
	if (*lpsz == 0)
		return 0;
	while(*lpsz != 0)
	{
		if((*lpsz) & 0x80) //korean
		{
			lpsz += 2;
		}
		else if ((*lpsz) == '\r' || (*lpsz) == '\n')
		{
			lpsz++;
			result += (_g_font_h + _g_font.linegap);
			if ((*lpsz) == '\r' || (*lpsz) == '\n')
			{
				result += (_g_font_h + _g_font.linegap);
				lpsz++;
			}
		}
		else
		{
			lpsz++;
		}	
	}
	return result;
}


int GpTextLenGet(const char * str)
{
	int result = 0;
	while (*str)
	{
		if (*str & 0x80)
		{
			str += 2;
		}
		else if (*str == '\r' || *str == '\n')
		{
			str++;
			if (result)
				result--;
		}
		else
			str++;
		result++;
	}
	return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
int _ham_ret_buf[2], _ham_ksuni_buf;
unsigned short _ham_comb_buf;

void _hautomaton_first0(char data1, char data2, int h_idx);
void _hautomaton_mid0(char data1, char data2, int h_idx);
void _hautomaton_mid1(char data1, char data2, int h_idx);
void _hautomaton_last0(char data1, char data2, int h_idx);
void _hautomaton_last1(char data1, char data2, int h_idx);

int _bin_src_at_hcode(void);
int _bin_src_at_hcode_ex(int flag, unsigned short h_code);
unsigned short _comb_first2last(unsigned short f_code);
unsigned short _h_mix_mid_mid(unsigned short h_code);
void _h_proc_last_first(unsigned short h_code, unsigned short *code_a, unsigned short * code_b);
void _h_proc_last_mid(unsigned short h_code, unsigned short * l_code, unsigned short * f_code);

typedef enum{
	_HAM_ST_FIRST0,
	_HAM_ST_MID0,
	_HAM_ST_MID1,
	_HAM_ST_LAST0,
	_HAM_ST_LAST1
}_HAUTOMATON_STATE;
_HAUTOMATON_STATE _hautomaton_state;

typedef void (*_hautomaton_fsm)(char data1, char data2, int h_idx);
_hautomaton_fsm _h_fsm_tbl[] = {
	_hautomaton_first0,
	_hautomaton_mid0,
	_hautomaton_mid1,
	_hautomaton_last0,
	_hautomaton_last1
};

#define _ExecuteHAutomaton(a, b, c) _h_fsm_tbl[_hautomaton_state](a, b, c)

void GpHAutomatonInit(void)
{
	_hautomaton_state = _HAM_ST_FIRST0;
	_ham_comb_buf = _ham_ksuni_buf = 0;
	_ham_ret_buf[0] = 0;
}

void _h_copy_return(int flag, char data1, char data2)
{
	char * p_ret, *p_ksuni;
	p_ret = (char *)&_ham_ret_buf[0];
	p_ksuni = (char *)&_ham_ksuni_buf;
	while (*p_ksuni)
		*p_ret++ = *p_ksuni++;
	if ( flag )
	{
		*p_ret++ = data1;
		*p_ret++ = data2;
	}
	*p_ret = 0;
}

char * GpHAutomatonBuffered(void)
{
	return (char*)(&_ham_ksuni_buf);
}

int GpHAutomatonInput(const char * p_input, char ** p_working)
{
	int idx;
	char data1, data2;
	
	_ham_ret_buf[0] = 0;
	
	if ( *p_input & 0x80 ) 	/* ks2601 hangul */
	{
		data1 = *p_input;
		data2 = *(p_input+1);
		
		if((data1 >= 0xB0) && (data1 <= 0xC8))	/* standard ks uni */
	 	{
	 		idx = (data1 - 176) * 94 + (data2 - 161);
	 		if ( idx >= 0 && idx < _KSUNISIZE )
	 			_h_copy_return(1, data1, data2);	 			
	 		else
	 			_h_copy_return(0, 0, 0);
	 		_ham_comb_buf = 0;
	 		_ham_ksuni_buf = 0;
	 		_hautomaton_state = _HAM_ST_FIRST0;
	 	}
	 	else if(data1 == 0xA4)	/* 확장 환성형 */
	 	{
	 		idx = data2 - 161;
	 		if ( idx >= 0 && idx < 94 && HCodeTableEx[idx] )
	 			_ExecuteHAutomaton(data1, data2, idx);
	 		else
	 		{
	 			_h_copy_return(0, 0, 0);
	 			_ham_comb_buf = 0;
	 			_ham_ksuni_buf = 0;
	 			_hautomaton_state = _HAM_ST_FIRST0;
	 		}
		}
		else
		{
			_h_copy_return(0, 0, 0);
			_ham_comb_buf = 0;
			_ham_ksuni_buf = 0;
			_hautomaton_state = _HAM_ST_FIRST0;
		}
	}
	else
	{
		data1 = *p_input;
		_h_copy_return(1, data1, 0);
		_ham_comb_buf = 0;
		_ham_ksuni_buf = 0;
		_hautomaton_state = _HAM_ST_FIRST0;
	}
	*p_working = (char *)&_ham_ret_buf[0];
	return (int)(gp_str_func.gpstrlen(*p_working));
}

void _h_separate_mid(unsigned short h_code, unsigned short * mid_a, unsigned short * mid_b)
{
	*mid_a = (h_code & ~0x3ff) | 0x1;
	*mid_b = 0x83a0;
	switch ( (0x1f & (h_code >> 5)) )
	{
	case 0x04:	//ㅐ
		*mid_a |= (0x03 << 5);
		break;
	case 0x06:	//ㅒ
		*mid_a |= (0x05 << 5);
		break;
	case 0x0a:	//ㅔ
		*mid_a |= (0x07 << 5);
		break;
	case 0x0c:	//ㅖ
		*mid_a |= (0x0b << 5);
		break;
	case 0x0e:	//ㅘ
		*mid_a |= (0x0d << 5);
		*mid_b = 0x8060;
		break;
	case 0x0f:	//ㅙ
		*mid_a |= (0x0d << 5);
		*mid_b = 0x8080;
		break;
	case 0x12:	//ㅚ
		*mid_a |= (0x0d << 5);
		break;
	case 0x15:	//ㅝ
		*mid_a |= (0x14 << 5);
		*mid_b = 0x80e0;
		break;
	case 0x16:	//ㅞ
		*mid_a |= (0x14 << 5);
		*mid_b = 0x8140;
		break;
	case 0x17:	//ㅟ
		*mid_a |= (0x14 << 5);
		break;
	case 0x1c:	//ㅢ
		*mid_a |= (0x1b << 5 );
	default:
		*mid_a |= (0x2 << 5);
		*mid_b = 0;
		break;
	}
}

void _h_separate_last(unsigned short h_code, unsigned short * code_a, unsigned short * code_b)
{
	unsigned short t_code = h_code & ~0x1f;
	*code_a = t_code;
	switch ( h_code & 0x1f )
	{
	case 0x04:	//ㄳ
		*code_a |= 0x02;
		*code_b = 0xac00;
		break;
	case 0x06:	//ㄵ
		*code_a |= 0x05;
		*code_b = 0xb800;
		break;
	case 0x07:	//ㄶ
		*code_a |= 0x05;
		*code_b = 0xd000;
		break;
	case 0x0a:	//ㄺ
		*code_a |= 0x09;
		*code_b = 0x8800;
		break;
	case 0x0b:	//ㄻ
		*code_a |= 0x09;
		*code_b = 0xa000;
		break;
	case 0x0c:	//ㄼ
		*code_a |= 0x09;
		*code_b = 0xa400;
		break;
	case 0x0d:	//ㄽ
		*code_a |= 0x09;
		*code_b = 0xac00;
		break;
	case 0x0e:	//ㄾ
		*code_a |= 0x09;
		*code_b = 0xc800;
		break;
	case 0x0f:	//ㄿ
		*code_a |= 0x09;
		*code_b = 0xcc00;
		break;
	case 0x10:	//ㅀ
		*code_a |= 0x09;
		*code_b = 0xd000;
		break;
	default:
		*code_a |= 0x1;
		*code_b = 0;
		break;
	}
}

int GpHAutomatonDelete(char ** p_working)
{
	unsigned short code_a, code_b;
	int ksuni_idx;
	
	*p_working = (char *)&_ham_ret_buf[0];
	switch ( _hautomaton_state )
	{
	case _HAM_ST_MID0:
		_ham_comb_buf = _ham_ksuni_buf = 0;
		_hautomaton_state = _HAM_ST_FIRST0;
		_h_copy_return(0, 0, 0);
		break;
	case _HAM_ST_MID1:	
		_h_separate_mid(_ham_comb_buf, &code_a, &code_b);
		if ( code_b )	/* 복모음 */
		{
			_ham_comb_buf = code_a;
			ksuni_idx = _bin_src_at_hcode_ex(0, (_ham_comb_buf & ~0x7c1f));
			_ham_ksuni_buf = ((161 + ksuni_idx % 94) << 8) | 0xa4;
			_hautomaton_state = _HAM_ST_MID1;
		}
		else			/* 단모음 */
		{
			_ham_comb_buf = _ham_ksuni_buf = 0;
			_hautomaton_state = _HAM_ST_FIRST0;
		}
		_h_copy_return(0, 0, 0);
		break;
	case _HAM_ST_LAST0:
		_h_separate_mid(_ham_comb_buf, &code_a, &code_b);
		if ( code_b )
		{
			_ham_comb_buf = code_a;
			ksuni_idx = _bin_src_at_hcode();
			_ham_ksuni_buf = ((161 + ksuni_idx % 94) << 8) | (176 + ksuni_idx / 94);
			_hautomaton_state = _HAM_ST_LAST0;
		}
		else
		{
			_ham_comb_buf = code_a;
			ksuni_idx = _bin_src_at_hcode_ex(1, (_ham_comb_buf & ~0x3ff));
			_ham_ksuni_buf = ((161 + ksuni_idx % 94) << 8) | 0xa4;
			_hautomaton_state = _HAM_ST_MID0;
		}
		_h_copy_return(0, 0, 0);
		break;
	case _HAM_ST_LAST1:
		_h_separate_last(_ham_comb_buf, &code_a, &code_b);
		if ( code_b )
		{
			_ham_comb_buf = code_a;
			if ( (ksuni_idx = _bin_src_at_hcode()) < 0 )
			{
				_ham_comb_buf = (code_a & ~0x1f) | 0x1;
				ksuni_idx = _bin_src_at_hcode();
				_ham_ksuni_buf = ((161 + ksuni_idx % 94) << 8) | (176 + ksuni_idx / 94);
				_hautomaton_state = _HAM_ST_LAST0;
			}
			else
			{
				_ham_ksuni_buf = ((161 + ksuni_idx % 94) << 8) | (176 + ksuni_idx / 94);
				_hautomaton_state = _HAM_ST_LAST1;
			}
		}
		else
		{
			_ham_comb_buf = code_a;
			ksuni_idx = _bin_src_at_hcode();
			_ham_ksuni_buf = ((161 + ksuni_idx % 94) << 8) | (176 + ksuni_idx / 94);
			_hautomaton_state = _HAM_ST_LAST0;
		}
		_h_copy_return(0, 0, 0);
		break;
	default:
		_ham_comb_buf = _ham_ksuni_buf = 0;
		_hautomaton_state = _HAM_ST_FIRST0;
		_h_copy_return(0, 0, 0);
		return -1;
	}
	return (int)(gp_str_func.gpstrlen(*p_working));
}

/* 초기 상태
	초성,중성이 올 수 있다
*/
void _hautomaton_first0(char data1, char data2, int h_idx)
{
	unsigned short h_code;
	
	h_code = HCodeTableEx[h_idx];
	_ham_ksuni_buf = (data2 << 8) | data1;
	_h_copy_return(0, 0, 0);
	if ( h_code & 0x1f )
	{
		_ham_comb_buf = (0x1 << 10) | (0x2 << 5) | h_code;
		_hautomaton_state = _HAM_ST_LAST1;
	}
	else if ( h_code & 0x3e0 )
	{
		_ham_comb_buf = (0x1 << 10) | h_code | (0x1);
		_hautomaton_state = _HAM_ST_MID1;
	}
	else
	{
		_ham_comb_buf = h_code | (0x2 << 5) | 0x1;
		_hautomaton_state = _HAM_ST_MID0;
	}
}

/* 초성만 입력된 상태
	중성이 올 수 있다
*/
void _hautomaton_mid0(char data1, char data2, int h_idx)
{
	int ksuni_idx;
	unsigned short h_code;
	
	h_code = HCodeTableEx[h_idx];
	
	if ( h_code & 0x7c00 )
	{
		_h_copy_return(1, data1, data2);
		_ham_comb_buf = h_code | (0x2 << 5) | 0x1;
		_ham_ksuni_buf = (data2 << 8) | data1;
		//_hautomaton_state = _HAM_ST_MID0;
	}
	else if ( h_code & 0x1f )	/* 종성이 오면 */
	{
		_h_copy_return(1, data1, data2);
		_ham_comb_buf = (0x1 << 10) | (0x2 << 5) | h_code;
		_ham_ksuni_buf = (data2 << 8) | data1;
		_hautomaton_state = _HAM_ST_LAST1;
	}
	else				/* 중성이 오는 경우 */
	{
		_ham_comb_buf = (_ham_comb_buf & ~0x3ff) | (0x3e0 & h_code) | 0x1;
		if ( (ksuni_idx = _bin_src_at_hcode()) < 0 )
		{
			/* KS2605에 없는 문자 입력 */
			_h_copy_return(1, data1, data2);
			_ham_comb_buf = h_code | (0x1 << 10) | 0x1;
			_ham_ksuni_buf = (data2 << 8) | (data1);
			_hautomaton_state = _HAM_ST_MID1;
		}
		else
		{
			/* KS2605에 있는 문자 입력 */
			data1 = 176 + ksuni_idx / 94;
			data2 = 161 + ksuni_idx % 94;
			_ham_ksuni_buf = (data2 << 8) | (data1);
			_h_copy_return(0, 0, 0);
			_hautomaton_state = _HAM_ST_LAST0;
		}
	}
}

/* 중성만 입력된 상태
	중성 결합이 일어날 수 있다
*/
void _hautomaton_mid1(char data1, char data2, int h_idx)
{
	int ksuni_idx;
	unsigned short h_code, mid_mix;
	
	h_code = HCodeTableEx[h_idx];
	
	if ( h_code & 0x1f )		/* 종성이 오면 */
	{
		_h_copy_return(1, data1, data2);
		_ham_comb_buf = (0x1 << 10) | (0x2 << 5) | h_code;
		_ham_ksuni_buf = (data2 << 8) | data1;
		_hautomaton_state = _HAM_ST_LAST1;
	}
	else if ( h_code & 0x3e0 ) 	/* 중성이 오면 */
	{
		mid_mix = _h_mix_mid_mid(h_code);
		if ( mid_mix )
		{
			/* 중성 + 중성 --> 중성 */
			_ham_comb_buf = (1 << 15) | (0x1 << 10) | mid_mix | 0x1;
			if ( (ksuni_idx = _bin_src_at_hcode_ex(0, (_ham_comb_buf & ~0x7c1f))) < 0 )
			{
				/* ks2605에 없는 문자일 경우 */
				_h_copy_return(1, data1, data2);
				_ham_comb_buf = (0x1 << 10) | h_code | 0x1;
				_ham_ksuni_buf = (data2 << 8) | data1;
				//_hautomaton_state = _HAM_ST_MID1;
			}
			else
			{
				/* ks2605에 있는 문자일 경우 */
				data1 = 0xa4;
				data2 = 161 + ksuni_idx % 94;
				_ham_ksuni_buf = (data2 << 8) | data1;
				_h_copy_return(0, 0, 0);
				//_hautomaton_state = _HAM_ST_MID1;
			}
		}
		else
		{
			/* 중성 + 중성 --> 중성 + 중성 */
			_h_copy_return(1, data1,data2);
			_ham_comb_buf = (0x1 << 10) | h_code | 0x1;
			_ham_ksuni_buf = (data2 << 8) | (data1);
			//_hautomaton_state = _HAM_ST_MID1;
		}
	}
	else
	{
		/* 중성 + 초성 --> 중성 + 초성 */
		_h_copy_return(1, data1, data2);
		_ham_comb_buf = h_code | (0x2 << 5) | 0x1;
		_ham_ksuni_buf = (data2 << 8) | (data1);
		_hautomaton_state = _HAM_ST_MID0;
	}
}

/*
초성과 중성이 입력된 상태
	초성,중성,종성 모두 올 수 있다
*/
void _hautomaton_last0(char data1, char data2, int h_idx)
{
	int ksuni_idx;
	unsigned short h_code, mid_mix, tmp_code;
	
	h_code = HCodeTableEx[h_idx];
	
	if ( h_code & 0x1f )		/* 종성이 오면 */
	{
		/*초성+중성+종성 --> 문자*/
		_ham_comb_buf = (_ham_comb_buf & ~0x1f) | (0x1f & h_code);
		if ( (ksuni_idx = _bin_src_at_hcode()) < 0 )
		{
			_h_copy_return(1, data1, data2);
			_ham_comb_buf = (0x1 << 10) | (0x2 << 5) | h_code;
			_ham_ksuni_buf = (data2 << 8) | data1;
			_hautomaton_state = _HAM_ST_LAST1;
		}
		else
		{
			data1 = 176 + ksuni_idx / 94;
			data2 = 161 + ksuni_idx % 94;
			_ham_ksuni_buf = (data2 << 8) | (data1);
			_h_copy_return(0, 0, 0);
			_hautomaton_state = _HAM_ST_LAST1;
		}
	}
	else if ( h_code & 0x3e0 ) 	/* 중성이 오면 */
	{
		/*초성 + 중성 + 중성 */
		mid_mix = _h_mix_mid_mid(h_code);
		if ( mid_mix )
		{
			/*중성 결합 발생 */
			_ham_comb_buf = (_ham_comb_buf & ~0x3ff) | mid_mix | 0x1;
			if ( (ksuni_idx = _bin_src_at_hcode()) < 0 )
			{
				_h_copy_return(1, data1, data2);
				_ham_comb_buf = (0x1 << 10) | h_code | 0x1;
				_ham_ksuni_buf = (data2 << 8) | data1;
				_hautomaton_state = _HAM_ST_MID1;
			}
			else
			{
				data1 = 176 + ksuni_idx / 94;
				data2 = 161 + ksuni_idx % 94;
				_ham_ksuni_buf = (data2 << 8) | data1;
				_h_copy_return(0, 0, 0);
				//_hautomaton_state = _HAM_ST_LAST0;
			}
		}
		else
		{
			_h_copy_return(1, data1, data2);
			_ham_comb_buf = (0x1 << 10) | h_code | 0x1;
			_ham_ksuni_buf = (data2 << 8) | data1;
			_hautomaton_state = _HAM_ST_MID1;
		}
	}
	else						/* 초성이 오면 */
	{
		tmp_code = _comb_first2last(h_code);	//초성 --> 종성 변환
		if ( !tmp_code )
		{
			_h_copy_return(1, data1, data2);
			_ham_comb_buf = h_code | (0x2 << 5) | 0x1;
			_ham_ksuni_buf = (data2 << 8) | data1;
			_hautomaton_state = _HAM_ST_MID0;
		}
		else
		{
			_ham_comb_buf = (_ham_comb_buf & ~0x1f) | (0x1f & tmp_code);
			if ( (ksuni_idx = _bin_src_at_hcode()) < 0 )
			{
				_h_copy_return(1, data1, data2);
				_ham_comb_buf = h_code | (0x2 << 5) | 0x1;
				_ham_ksuni_buf = (data2 << 8) | data1;
				_hautomaton_state = _HAM_ST_MID0;
			}
			else
			{
				data1 = 176 + ksuni_idx / 94;
				data2 = 161 + ksuni_idx % 94;
				_ham_ksuni_buf = (data2 << 8) | data1;
				_h_copy_return(0, 0, 0);
				_hautomaton_state = _HAM_ST_LAST1;
			}
		}
	}
}

/*
초성 + 중성 + 종성이 결합된 상태
	종성 결합을 체크하면 된다
*/
void _hautomaton_last1(char data1, char data2, int h_idx)
{
	int ksuni_idx, ksuni_idx_b;
	unsigned short h_code, code_a, code_b;
	
	h_code = HCodeTableEx[h_idx];
	
	if ( h_code & 0x1f )		/* 종성이 오면 */
	{
		_h_copy_return(1, data1,data2);
		_ham_comb_buf = (0x1 << 10) | (0x2 << 5) | h_code;
		_ham_ksuni_buf = (data2 << 8) | data1;
		_hautomaton_state = _HAM_ST_MID0;
	}
	else if ( h_code & 0x3e0 ) 	/* 중성이 오면 */
	{
		_h_proc_last_mid(h_code, &code_a, &code_b);
		if ( !code_b )
		{
			_h_copy_return(1, data1, data2);
			_ham_comb_buf = (0x1 << 10) | h_code | 0x1;
			_ham_ksuni_buf = (data2 << 8) | data1;
			_hautomaton_state = _HAM_ST_MID1;
		}
		else
		{
			_ham_comb_buf = code_b | 0x1;
			if ( (ksuni_idx = _bin_src_at_hcode()) < 0 )
			{
				_h_copy_return(1, data1, data2);
				_ham_comb_buf = (0x1 << 10) | h_code | 0x1;
				_ham_ksuni_buf = (data2 << 8) | data1;
				_hautomaton_state = _HAM_ST_MID1;
			}
			else
			{
				ksuni_idx_b = ksuni_idx;
				_ham_comb_buf = code_a;
				if ( (ksuni_idx = _bin_src_at_hcode()) < 0 )
				{
					//this is impossible
					_h_copy_return(1, data1, data2);
					_ham_comb_buf = (0x1 << 10) | h_code | 0x1;
					_ham_ksuni_buf = (data2 << 8) | data1;
					_hautomaton_state = _HAM_ST_MID1;
				}
				else
				{
					data1 = 176 + (ksuni_idx) / 94;
					data2 = 161 + (ksuni_idx) % 94;
					_ham_ksuni_buf = (data2 << 8) | data1;
					data1 = 176 + (ksuni_idx_b) / 94;
					data2 = 161 + (ksuni_idx_b) % 94;
					_h_copy_return(1, data1, data2);
					_ham_comb_buf = code_b | 0x1;
					_ham_ksuni_buf = (data2 << 8) | data1;
					_hautomaton_state = _HAM_ST_LAST0;
				}
			}
		}
	}
	else						/* 초성이 오면 */
	{
		_h_proc_last_first(h_code, &code_a, &code_b);
		if ( code_b )	/* 초성이 새로운 문자를 만드는 경우 */
		{
			_h_copy_return(1, data1, data2);
			_ham_comb_buf = code_b;
			_ham_ksuni_buf = (data2 << 8) | data1;
			_hautomaton_state = _HAM_ST_MID0;
		}
		else	/* 초성이 이전 문자의 종성에 결합된 경우 */
		{
			_ham_comb_buf = code_a;
			if ( (ksuni_idx = _bin_src_at_hcode()) < 0 )
			{
				//ks2605에 없는 문자
				_h_copy_return(1, data1, data2);
				_ham_comb_buf = h_code | (0x2 << 5) | 0x1;
				_ham_ksuni_buf = (data2 << 8) | data1;
				_hautomaton_state = _HAM_ST_MID0;
			}
			else
			{
				data1 = 176 + ksuni_idx / 94;
				data2 = 161 + ksuni_idx % 94;
				_ham_ksuni_buf = (data2 << 8) | data1;
				_h_copy_return(0, 0, 0);
				//_hautomaton_state = _HAM_ST_LAST1;
			}
		}	
	}
}

void _h_proc_last_first(unsigned short h_code, unsigned short *code_a, unsigned short * code_b)
{
	*code_a = _ham_comb_buf & ~0x1f;
	*code_b = 0;
	switch ( _ham_comb_buf & 0x1f)
	{
	case 0x02:	//ㄱ
		if ( h_code == 0xac00 )	//ㅅ	==> ㄳ
			*code_a |= 0x04;
		else
		{
			*code_a = _ham_comb_buf;
			*code_b = h_code | (0x2 << 5) | 0x1;
		}
		break;
	case 0x05:	//ㄴ
		if ( h_code == 0xb800 )	//ㅈ ==> ㄵ
			*code_a |= 0x06;
		else if ( h_code == 0xd000) //ㅎ ==> ㄶ
			*code_a |= 0x07;
		else
		{
			*code_a = _ham_comb_buf;
			*code_b = h_code | (0x2 << 5) | 0x1;
		}
		break;
	case 0x09:	//ㄹ
		switch ( h_code )
		{
		case 0x8800:	//ㄱ ==> ㄺ
			*code_a |= 0x0a;
			break;
		case 0xa000:	//ㅁ ==> ㄻ
			*code_a |= 0x0b;
			break;
		case 0xa400:	//ㅂ ==> ㄼ
			*code_a |= 0x0c;
			break;
		case 0xac00:	//ㅅ ==> ㄽ
			*code_a |= 0x0d;
			break;
		case 0xc800:	//ㅌ ==> ㄾ
			*code_a |= 0x0e;
			break;
		case 0xcc00:	//ㅍ ==> ㄿ
			*code_a |= 0x0f;
			break;
		case 0xd000:	//ㅎ ==> ㅀ
			*code_a |= 0x10;
			break;
		default:
			*code_a = _ham_comb_buf;
			*code_b = h_code | (0x2 << 5) | 0x1;
			break;
		}
		break;
	case 0x13:	//ㅂ
		if ( h_code == 0xac00 )	//ㅅ ==> ㅄ
			*code_a |= 0x14;
		else
		{
			*code_a = _ham_comb_buf;
			*code_b = h_code | (0x2 << 5) | 0x1;
		}
		break;
	default:
		*code_a = _ham_comb_buf;
		*code_b = h_code | (0x2 << 5) | 0x1;
	}
}

void _h_proc_last_mid(unsigned short h_code, unsigned short * l_code, unsigned short * f_code)
{
	unsigned short old_code = 0x1f & _ham_comb_buf;
	unsigned short tmp_code = _ham_comb_buf & ~0x1f;
	
	*f_code = h_code;	//중성
	switch ( old_code )
	{
	case 0x04:	//ㄳ
		*l_code = tmp_code | 0x02;
		*f_code |= 0xac00;
		break;
	case 0x06:	//ㄵ
		*l_code = tmp_code | 0x05;
		*f_code |= 0xb800;
		break;
	case 0x07:	//ㄶ
		*l_code = tmp_code | 0x05;
		*f_code |= 0xd000;
		break;
	case 0x0a:	//ㄺ
		*l_code = tmp_code | 0x09;
		*f_code |= 0x8800;
		break;
	case 0x0b:	//ㄻ
		*l_code = tmp_code | 0x09;
		*f_code |= 0xa000;
		break;
	case 0x0c:	//ㄼ
		*l_code = tmp_code | 0x09;
		*f_code |= 0xa400;
		break;
	case 0x0d:	//ㄽ
		*l_code = tmp_code | 0x09;
		*f_code |= 0xac00;
		break;
	case 0x0e:	//ㄾ
		*l_code = tmp_code | 0x09;
		*f_code |= 0xc800;
		break;
	case 0x0f:	//ㄿ
		*l_code = tmp_code | 0x09;
		*f_code |= 0xcc00;
		break;
	case 0x10:	//ㅀ
		*l_code = tmp_code | 0x09;
		*f_code |= 0xd000;
		break;
	case 0x14:	//ㅄ
		*l_code = tmp_code | 0x13;
		*f_code |= 0xac00;
		break;
	default:
		*l_code = tmp_code | 0x1;
		switch ( old_code )
		{
		case 0x02:
			*f_code |= 0x8800;
			break;
		case 0x03:	
			*f_code |= 0x8c00;
			break;
		case 0x05:
			*f_code |= 0x9000;
			break;
		case 0x08:
			*f_code |= 0x9400;
			break;
		case 0x09:
			*f_code |= 0x9c00;	
			break;
		case 0x11:
			*f_code |= 0xa000;
			break;
		case 0x13:
			*f_code |= 0xa400;
			break;
		case 0x15:
			*f_code |= 0xac00;
			break;
		case 0x16:
			*f_code |= 0xb000;
			break;
		case 0x17:
			*f_code |= 0xb400;
			break;
		case 0x18:
			*f_code |= 0xb800;
			break;
		case 0x19:
			*f_code |= 0xc000;
			break;
		case 0x1a:
			*f_code |= 0xc400;
			break;
		case 0x1b:
			*f_code |= 0xc800;
			break;
		case 0x1c:
			*f_code |= 0xcc00;
			break;
		case 0x1d:
			*f_code |= 0xd000;
			break;
		default:
			*l_code = _ham_comb_buf;
			*f_code = 0;
			break;
		}
		break;
	}
}

unsigned short _comb_first2last(unsigned short f_code)
{
	switch ( f_code )
	{
	case 0x8800:	return 0x8002;	//ㄱ
	case 0x8c00:	return 0x8003;	//ㄲ
	case 0x9000:	return 0x8005;	//ㄴ
	case 0x9400:	return 0x8008;	//ㄷ
	case 0x9c00:	return 0x8009;	//ㄹ
	case 0xa000:	return 0x8011;	//ㅁ
	case 0xa400:	return 0x8013;	//ㅂ
	case 0xac00:	return 0x8015;	//ㅅ
	case 0xb000:	return 0x8016;	//ㅆ
	case 0xb400:	return 0x8017;	//ㅇ
	case 0xb800:	return 0x8018;	//ㅈ
	case 0xc000:	return 0x8019;	//ㅊ
	case 0xc400:	return 0x801a;	//ㅋ
	case 0xc800:	return 0x801b;	//ㅌ
	case 0xcc00:	return 0x801c;	//ㅍ
	case 0xd000:	return 0x801d;	//ㅎ
	default:		return 0;
	//case 0x9800:	return 0;		//ㄸ
	//case 0xa800:	return 0;		//ㅃ
	//case 0xbc00:	return 0;		//ㅉ
		break;
	}
	return 0;
}

int _bin_src_at_hcode(void)
{
	int lower = 0, mid, upper;
	//int tb_size = _KSUNISIZE;
	
	upper = _KSUNISIZE -1;
	
	if ( _ham_comb_buf < HCodeTable[lower] )
		return -1;
	if ( _ham_comb_buf > HCodeTable[upper] )
		return -1;
	
	while( lower <= upper )
	{
		mid = (upper+lower) >> 1;

		if ( HCodeTable[mid] < _ham_comb_buf )
			lower  = mid + 1;
		else if ( HCodeTable[mid] > _ham_comb_buf )
			upper = mid -1;
		else
			break;
	}
	if ( lower > upper )
		return -1;
	return mid;
}

int _bin_src_at_hcode_ex(int flag, unsigned short h_code)
{
	int lower, mid, upper;
	
	if ( flag )
	{
		for (mid = 0; mid<94; mid++)
		{
			if ( !HCodeTableEx[mid] ) return -1;
			if ( h_code == HCodeTableEx[mid] ) break;
		}
	}
	else
	{
		if ( h_code < 0x8060 )	/* ㅏ */
			return -1;
		if ( h_code > 0x83a0 ) /* ㅣ */
			return -1;
		lower = 30;	/* ㅏ */
		upper = 50;	/* ㅣ */
		while (lower <= upper )
		{
			mid = (upper + lower) >> 1;
			if ( HCodeTableEx[mid] < _ham_comb_buf )
				lower = mid + 1;
			else if ( HCodeTableEx[mid] > _ham_comb_buf )
				upper = mid -1;
			else
				break;
		}
		if ( lower > upper )
			return -1;
	}
	return mid;
}

/* 중성 결합 체크
	step 0. 중성 요소만 결합 검사
	step 1. 초성이 있는지 여부 검사
*/
unsigned short _h_mix_mid_mid(unsigned short h_code)
{
	unsigned short mid_old, mid_new, mid_mix;
	
	mid_old = (_ham_comb_buf >> 5) & 0x1f;
	mid_new = (h_code >> 5 ) & 0x1f;	
	mid_mix = 0;
	
	switch ( mid_old )
	{
	case 0x3:			//ㅏ
		if ( mid_new == 0x1d )	//ㅏ + ㅣ = ㅐ
			mid_mix = 0x4;
		break;
	case 0x5:			//ㅑ
		if ( mid_new == 0x1d )	//ㅑ + ㅣ = ㅒ
			mid_mix = 0x6;
		break;
	case 0x7:			//ㅓ
		if ( mid_new == 0x1d ) 	//ㅓ + ㅣ = ㅔ
			mid_mix = 0xa;
		break;
	case 0xb:			//ㅕ
		if ( mid_new == 0x1d ) 	//ㅕ + ㅣ = ㅖ
			mid_mix = 0xc;
		break;
	case 0xd:			//ㅗ
		switch ( mid_new )
		{
		case 0x1d:	//ㅗ + ㅣ = ㅚ
			mid_mix = 0x12;
			break;
		case 0x3:		//ㅗ + ㅏ = ㅘ
			mid_mix = 0xe;
			break;
		case 0x4:		//ㅗ + ㅐ = ㅙ
			mid_mix = 0xf;
			break;
		}
		break;
	case 0x14:		//ㅜ
		switch ( mid_new ) //ㅣㅓㅔ
		{
		case 0x1d:	//ㅜ + ㅣ = ㅟ
			mid_mix = 0x17;
			break;
		case 0x7:		//ㅜ + ㅓ = ㅝ
			mid_mix = 0x15;
			break;
		case 0xa:		//ㅜ + ㅔ = ㅞ
			mid_mix = 0x16;
			break;
		}
		break;
	case 0x1b:		//ㅡ
		if ( mid_new == 0x1d )	//ㅡ + ㅣ = ㅢ
			mid_mix = 0x1d;
		break;
	default:
		break;
	}
	
	mid_mix <<= 5; 
	return mid_mix;
}
