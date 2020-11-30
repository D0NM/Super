#include "gpstdio.h"
#include "gpos_internal.h"

#define FIO_ENTERCRITICAL()		_gp_os_res_lock(GPOS_RESID_FIO)
#define FIO_EXITCRITICAL()		_gp_os_res_unlock(GPOS_RESID_FIO)
ERR_CODE GpSmcIDGet(unsigned char* data)
{
	int ret;
	FIO_ENTERCRITICAL();
	ret = _gp_smc_id_get(data);
	FIO_EXITCRITICAL();
	return (ERR_CODE)ret;
}

ERR_CODE GpE2PROMRead(int pos, int count, unsigned char * dest)
{
	int ret;
	FIO_ENTERCRITICAL();
	ret = _gp_e2prom_read(pos, count, dest);
	FIO_EXITCRITICAL();
	return (ERR_CODE)ret;
}

ERR_CODE GpE2PROMWrite(int pos, int count, unsigned char * src)
{
	int ret;
	FIO_ENTERCRITICAL();
	ret = _gp_e2prom_write(pos, count, src);
	FIO_EXITCRITICAL();
	return (ERR_CODE)ret;
}
ERR_CODE GpDeviceIDGet(unsigned char * data)
{
	int err;
	FIO_ENTERCRITICAL();
	err = _gp_dev_id_get(data);
	FIO_EXITCRITICAL();
	return (ERR_CODE)err;
}

ERR_CODE GpDevUserInfoGet(int mode, unsigned char * data)
{
	char sn_encode_stream[8] = {"SANGHYUK"};
	int i,j;
	ERR_CODE err;
	
	if(select == 1)
		err = GpE2PROMRead(DEVICE_UID_POS, DEVICE_SN_LEN, data);
	else if(select == 2)	
		err = GpE2PROMRead(DEVICE_UID_POS+16, DEVICE_SN_LEN, data);
	if ( err == SM_OK )
	{
		j = 0;
		for ( i=0; i<DEVICE_SN_LEN; i++ )
		{
			data[i] ^= sn_encode_stream[j++];
			j %= sizeof(sn_encode_stream);
		}
	}
	return err;
}
ERR_CODE GpDevUserInfoSet(int mode,unsigned char * data)
{
	char sn_encode_stream[8] = {"SANGHYUK"};
	int i,j;	
	ERR_CODE err;
	
	j = 0;
	for ( i=0; i<DEVICE_SN_LEN; i++ )
	{
		data[i] ^= sn_encode_stream[j++];
		j %= sizeof(sn_encode_stream);
	}
	
	if(select == 1)
		err = GpE2PROMWrite(DEVICE_UID_POS, DEVICE_SN_LEN, data);
	else if(select == 2)	
		err = GpE2PROMWrite(DEVICE_UID_POS+16, DEVICE_SN_LEN, data);
	
	return err;	
	
}