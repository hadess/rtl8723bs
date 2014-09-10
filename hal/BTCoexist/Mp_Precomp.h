#ifndef	__MP_PRECOMP_H__
#define __MP_PRECOMP_H__
//For follow windows code.

#include "halbt_precomp.h"
#include <HalcomPhyReg.h>

#define BT_30_SUPPORT 0

#ifdef CONFIG_BT_COEXIST
#undef BT_30_SUPPORT
#define BT_30_SUPPORT 1
#endif

#undef TRUE
#define TRUE		1

#undef FALSE
#define FALSE		0

#define delay_ms(_t)	rtw_mdelay_os(_t)

#endif