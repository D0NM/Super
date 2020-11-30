#ifndef __gpgraphic_internal_h__
#define __gpgraphic_internal_h__

#define GPC_FLIP_SLOW	0
#define GPC_FLIP_MID	1
#define GPC_FLIP_FAST	2
#define GPC_FLIP_DEFAULT	GPC_FLIP_MID

void GpFlipModeSet(int mode);								/* internal */

#endif /* __gpgraphic_internal_h__ */