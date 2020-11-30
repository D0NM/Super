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
	 	else if(data1 == 0xA4)	/* Ȯ�� ȯ���� */
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
	case 0x04:	//��
		*mid_a |= (0x03 << 5);
		break;
	case 0x06:	//��
		*mid_a |= (0x05 << 5);
		break;
	case 0x0a:	//��
		*mid_a |= (0x07 << 5);
		break;
	case 0x0c:	//��
		*mid_a |= (0x0b << 5);
		break;
	case 0x0e:	//��
		*mid_a |= (0x0d << 5);
		*mid_b = 0x8060;
		break;
	case 0x0f:	//��
		*mid_a |= (0x0d << 5);
		*mid_b = 0x8080;
		break;
	case 0x12:	//��
		*mid_a |= (0x0d << 5);
		break;
	case 0x15:	//��
		*mid_a |= (0x14 << 5);
		*mid_b = 0x80e0;
		break;
	case 0x16:	//��
		*mid_a |= (0x14 << 5);
		*mid_b = 0x8140;
		break;
	case 0x17:	//��
		*mid_a |= (0x14 << 5);
		break;
	case 0x1c:	//��
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
	case 0x04:	//��
		*code_a |= 0x02;
		*code_b = 0xac00;
		break;
	case 0x06:	//��
		*code_a |= 0x05;
		*code_b = 0xb800;
		break;
	case 0x07:	//��
		*code_a |= 0x05;
		*code_b = 0xd000;
		break;
	case 0x0a:	//��
		*code_a |= 0x09;
		*code_b = 0x8800;
		break;
	case 0x0b:	//��
		*code_a |= 0x09;
		*code_b = 0xa000;
		break;
	case 0x0c:	//��
		*code_a |= 0x09;
		*code_b = 0xa400;
		break;
	case 0x0d:	//��
		*code_a |= 0x09;
		*code_b = 0xac00;
		break;
	case 0x0e:	//��
		*code_a |= 0x09;
		*code_b = 0xc800;
		break;
	case 0x0f:	//��
		*code_a |= 0x09;
		*code_b = 0xcc00;
		break;
	case 0x10:	//��
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
		if ( code_b )	/* ������ */
		{
			_ham_comb_buf = code_a;
			ksuni_idx = _bin_src_at_hcode_ex(0, (_ham_comb_buf & ~0x7c1f));
			_ham_ksuni_buf = ((161 + ksuni_idx % 94) << 8) | 0xa4;
			_hautomaton_state = _HAM_ST_MID1;
		}
		else			/* �ܸ��� */
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

/* �ʱ� ����
	�ʼ�,�߼��� �� �� �ִ�
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

/* �ʼ��� �Էµ� ����
	�߼��� �� �� �ִ�
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
	else if ( h_code & 0x1f )	/* ������ ���� */
	{
		_h_copy_return(1, data1, data2);
		_ham_comb_buf = (0x1 << 10) | (0x2 << 5) | h_code;
		_ham_ksuni_buf = (data2 << 8) | data1;
		_hautomaton_state = _HAM_ST_LAST1;
	}
	else				/* �߼��� ���� ��� */
	{
		_ham_comb_buf = (_ham_comb_buf & ~0x3ff) | (0x3e0 & h_code) | 0x1;
		if ( (ksuni_idx = _bin_src_at_hcode()) < 0 )
		{
			/* KS2605�� ���� ���� �Է� */
			_h_copy_return(1, data1, data2);
			_ham_comb_buf = h_code | (0x1 << 10) | 0x1;
			_ham_ksuni_buf = (data2 << 8) | (data1);
			_hautomaton_state = _HAM_ST_MID1;
		}
		else
		{
			/* KS2605�� �ִ� ���� �Է� */
			data1 = 176 + ksuni_idx / 94;
			data2 = 161 + ksuni_idx % 94;
			_ham_ksuni_buf = (data2 << 8) | (data1);
			_h_copy_return(0, 0, 0);
			_hautomaton_state = _HAM_ST_LAST0;
		}
	}
}

/* �߼��� �Էµ� ����
	�߼� ������ �Ͼ �� �ִ�
*/
void _hautomaton_mid1(char data1, char data2, int h_idx)
{
	int ksuni_idx;
	unsigned short h_code, mid_mix;
	
	h_code = HCodeTableEx[h_idx];
	
	if ( h_code & 0x1f )		/* ������ ���� */
	{
		_h_copy_return(1, data1, data2);
		_ham_comb_buf = (0x1 << 10) | (0x2 << 5) | h_code;
		_ham_ksuni_buf = (data2 << 8) | data1;
		_hautomaton_state = _HAM_ST_LAST1;
	}
	else if ( h_code & 0x3e0 ) 	/* �߼��� ���� */
	{
		mid_mix = _h_mix_mid_mid(h_code);
		if ( mid_mix )
		{
			/* �߼� + �߼� --> �߼� */
			_ham_comb_buf = (1 << 15) | (0x1 << 10) | mid_mix | 0x1;
			if ( (ksuni_idx = _bin_src_at_hcode_ex(0, (_ham_comb_buf & ~0x7c1f))) < 0 )
			{
				/* ks2605�� ���� ������ ��� */
				_h_copy_return(1, data1, data2);
				_ham_comb_buf = (0x1 << 10) | h_code | 0x1;
				_ham_ksuni_buf = (data2 << 8) | data1;
				//_hautomaton_state = _HAM_ST_MID1;
			}
			else
			{
				/* ks2605�� �ִ� ������ ��� */
				data1 = 0xa4;
				data2 = 161 + ksuni_idx % 94;
				_ham_ksuni_buf = (data2 << 8) | data1;
				_h_copy_return(0, 0, 0);
				//_hautomaton_state = _HAM_ST_MID1;
			}
		}
		else
		{
			/* �߼� + �߼� --> �߼� + �߼� */
			_h_copy_return(1, data1,data2);
			_ham_comb_buf = (0x1 << 10) | h_code | 0x1;
			_ham_ksuni_buf = (data2 << 8) | (data1);
			//_hautomaton_state = _HAM_ST_MID1;
		}
	}
	else
	{
		/* �߼� + �ʼ� --> �߼� + �ʼ� */
		_h_copy_return(1, data1, data2);
		_ham_comb_buf = h_code | (0x2 << 5) | 0x1;
		_ham_ksuni_buf = (data2 << 8) | (data1);
		_hautomaton_state = _HAM_ST_MID0;
	}
}

/*
�ʼ��� �߼��� �Էµ� ����
	�ʼ�,�߼�,���� ��� �� �� �ִ�
*/
void _hautomaton_last0(char data1, char data2, int h_idx)
{
	int ksuni_idx;
	unsigned short h_code, mid_mix, tmp_code;
	
	h_code = HCodeTableEx[h_idx];
	
	if ( h_code & 0x1f )		/* ������ ���� */
	{
		/*�ʼ�+�߼�+���� --> ����*/
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
	else if ( h_code & 0x3e0 ) 	/* �߼��� ���� */
	{
		/*�ʼ� + �߼� + �߼� */
		mid_mix = _h_mix_mid_mid(h_code);
		if ( mid_mix )
		{
			/*�߼� ���� �߻� */
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
	else						/* �ʼ��� ���� */
	{
		tmp_code = _comb_first2last(h_code);	//�ʼ� --> ���� ��ȯ
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
�ʼ� + �߼� + ������ ���յ� ����
	���� ������ üũ�ϸ� �ȴ�
*/
void _hautomaton_last1(char data1, char data2, int h_idx)
{
	int ksuni_idx, ksuni_idx_b;
	unsigned short h_code, code_a, code_b;
	
	h_code = HCodeTableEx[h_idx];
	
	if ( h_code & 0x1f )		/* ������ ���� */
	{
		_h_copy_return(1, data1,data2);
		_ham_comb_buf = (0x1 << 10) | (0x2 << 5) | h_code;
		_ham_ksuni_buf = (data2 << 8) | data1;
		_hautomaton_state = _HAM_ST_MID0;
	}
	else if ( h_code & 0x3e0 ) 	/* �߼��� ���� */
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
	else						/* �ʼ��� ���� */
	{
		_h_proc_last_first(h_code, &code_a, &code_b);
		if ( code_b )	/* �ʼ��� ���ο� ���ڸ� ����� ��� */
		{
			_h_copy_return(1, data1, data2);
			_ham_comb_buf = code_b;
			_ham_ksuni_buf = (data2 << 8) | data1;
			_hautomaton_state = _HAM_ST_MID0;
		}
		else	/* �ʼ��� ���� ������ ������ ���յ� ��� */
		{
			_ham_comb_buf = code_a;
			if ( (ksuni_idx = _bin_src_at_hcode()) < 0 )
			{
				//ks2605�� ���� ����
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
	case 0x02:	//��
		if ( h_code == 0xac00 )	//��	==> ��
			*code_a |= 0x04;
		else
		{
			*code_a = _ham_comb_buf;
			*code_b = h_code | (0x2 << 5) | 0x1;
		}
		break;
	case 0x05:	//��
		if ( h_code == 0xb800 )	//�� ==> ��
			*code_a |= 0x06;
		else if ( h_code == 0xd000) //�� ==> ��
			*code_a |= 0x07;
		else
		{
			*code_a = _ham_comb_buf;
			*code_b = h_code | (0x2 << 5) | 0x1;
		}
		break;
	case 0x09:	//��
		switch ( h_code )
		{
		case 0x8800:	//�� ==> ��
			*code_a |= 0x0a;
			break;
		case 0xa000:	//�� ==> ��
			*code_a |= 0x0b;
			break;
		case 0xa400:	//�� ==> ��
			*code_a |= 0x0c;
			break;
		case 0xac00:	//�� ==> ��
			*code_a |= 0x0d;
			break;
		case 0xc800:	//�� ==> ��
			*code_a |= 0x0e;
			break;
		case 0xcc00:	//�� ==> ��
			*code_a |= 0x0f;
			break;
		case 0xd000:	//�� ==> ��
			*code_a |= 0x10;
			break;
		default:
			*code_a = _ham_comb_buf;
			*code_b = h_code | (0x2 << 5) | 0x1;
			break;
		}
		break;
	case 0x13:	//��
		if ( h_code == 0xac00 )	//�� ==> ��
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
	
	*f_code = h_code;	//�߼�
	switch ( old_code )
	{
	case 0x04:	//��
		*l_code = tmp_code | 0x02;
		*f_code |= 0xac00;
		break;
	case 0x06:	//��
		*l_code = tmp_code | 0x05;
		*f_code |= 0xb800;
		break;
	case 0x07:	//��
		*l_code = tmp_code | 0x05;
		*f_code |= 0xd000;
		break;
	case 0x0a:	//��
		*l_code = tmp_code | 0x09;
		*f_code |= 0x8800;
		break;
	case 0x0b:	//��
		*l_code = tmp_code | 0x09;
		*f_code |= 0xa000;
		break;
	case 0x0c:	//��
		*l_code = tmp_code | 0x09;
		*f_code |= 0xa400;
		break;
	case 0x0d:	//��
		*l_code = tmp_code | 0x09;
		*f_code |= 0xac00;
		break;
	case 0x0e:	//��
		*l_code = tmp_code | 0x09;
		*f_code |= 0xc800;
		break;
	case 0x0f:	//��
		*l_code = tmp_code | 0x09;
		*f_code |= 0xcc00;
		break;
	case 0x10:	//��
		*l_code = tmp_code | 0x09;
		*f_code |= 0xd000;
		break;
	case 0x14:	//��
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
	case 0x8800:	return 0x8002;	//��
	case 0x8c00:	return 0x8003;	//��
	case 0x9000:	return 0x8005;	//��
	case 0x9400:	return 0x8008;	//��
	case 0x9c00:	return 0x8009;	//��
	case 0xa000:	return 0x8011;	//��
	case 0xa400:	return 0x8013;	//��
	case 0xac00:	return 0x8015;	//��
	case 0xb000:	return 0x8016;	//��
	case 0xb400:	return 0x8017;	//��
	case 0xb800:	return 0x8018;	//��
	case 0xc000:	return 0x8019;	//��
	case 0xc400:	return 0x801a;	//��
	case 0xc800:	return 0x801b;	//��
	case 0xcc00:	return 0x801c;	//��
	case 0xd000:	return 0x801d;	//��
	default:		return 0;
	//case 0x9800:	return 0;		//��
	//case 0xa800:	return 0;		//��
	//case 0xbc00:	return 0;		//��
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
		if ( h_code < 0x8060 )	/* �� */
			return -1;
		if ( h_code > 0x83a0 ) /* �� */
			return -1;
		lower = 30;	/* �� */
		upper = 50;	/* �� */
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

/* �߼� ���� üũ
	step 0. �߼� ��Ҹ� ���� �˻�
	step 1. �ʼ��� �ִ��� ���� �˻�
*/
unsigned short _h_mix_mid_mid(unsigned short h_code)
{
	unsigned short mid_old, mid_new, mid_mix;
	
	mid_old = (_ham_comb_buf >> 5) & 0x1f;
	mid_new = (h_code >> 5 ) & 0x1f;	
	mid_mix = 0;
	
	switch ( mid_old )
	{
	case 0x3:			//��
		if ( mid_new == 0x1d )	//�� + �� = ��
			mid_mix = 0x4;
		break;
	case 0x5:			//��
		if ( mid_new == 0x1d )	//�� + �� = ��
			mid_mix = 0x6;
		break;
	case 0x7:			//��
		if ( mid_new == 0x1d ) 	//�� + �� = ��
			mid_mix = 0xa;
		break;
	case 0xb:			//��
		if ( mid_new == 0x1d ) 	//�� + �� = ��
			mid_mix = 0xc;
		break;
	case 0xd:			//��
		switch ( mid_new )
		{
		case 0x1d:	//�� + �� = ��
			mid_mix = 0x12;
			break;
		case 0x3:		//�� + �� = ��
			mid_mix = 0xe;
			break;
		case 0x4:		//�� + �� = ��
			mid_mix = 0xf;
			break;
		}
		break;
	case 0x14:		//��
		switch ( mid_new ) //�Ӥä�
		{
		case 0x1d:	//�� + �� = ��
			mid_mix = 0x17;
			break;
		case 0x7:		//�� + �� = ��
			mid_mix = 0x15;
			break;
		case 0xa:		//�� + �� = ��
			mid_mix = 0x16;
			break;
		}
		break;
	case 0x1b:		//��
		if ( mid_new == 0x1d )	//�� + �� = ��
			mid_mix = 0x1d;
		break;
	default:
		break;
	}
	
	mid_mix <<= 5; 
	return mid_mix;
}
