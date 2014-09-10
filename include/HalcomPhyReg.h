#ifndef	__HALCOMPHYREG_H__
#define __HALCOMPHYREG_H__

//for PutRegsetting & GetRegSetting BitMask
#define		bMaskByte0                		0xff	// Reg 0xc50 rOFDM0_XAAGCCore~0xC6f
#define		bMaskByte1                		0xff00
#define		bMaskByte2                		0xff0000
#define		bMaskByte3                		0xff000000
#define		bMaskHWord                		0xffff0000
#define		bMaskLWord                		0x0000ffff
#define		bMaskDWord                		0xffffffff
#define		bMask12Bits				0xfff
#define		bMaskH4Bits				0xf0000000
#define 		bMaskOFDM_D			0xffc00000
#define		bMaskCCK				0x3f3f3f3f

//for PutRFRegsetting & GetRFRegSetting BitMask
//#define		bMask12Bits               0xfffff	// RF Reg mask bits
//#define		bMask20Bits               0xfffff	// RF Reg mask bits T65 RF
#define 		bRFRegOffsetMask			0xfffff

//#define		bEnable                   0x1	// Useless
//#define		bDisable                  0x0

#define		LeftAntenna		0x0	// Useless
#define		RightAntenna	0x1

#define		tCheckTxStatus		500   //500ms // Useless
#define		tUpdateRxCounter	100   //100ms

#define		rateCCK		0	// Useless
#define		rateOFDM	1
#define		rateHT		2


#define		PathA                     			0x0	// Useless
#define		PathB                     			0x1
#define		PathC                     			0x2
#define		PathD                     			0x3

#endif //End __HALCOMPHYREG_H__