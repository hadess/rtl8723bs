/******************************************************************************
 *
 * Copyright(c) 2007 - 2011 Realtek Corporation. All rights reserved.
 *                                        
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110, USA
 *
 *
 ******************************************************************************/

//============================================================
// include files
//============================================================

#include "odm_precomp.h"


static const u2Byte dB_Invert_Table[8][12] = {
	{	1,		1,		1,		2,		2,		2,		2,		3,		3,		3,		4,		4},
	{	4,		5,		6,		6,		7,		8,		9,		10,		11,		13,		14,		16},
	{	18,		20,		22,		25,		28,		32,		35,		40,		45,		50,		56,		63},
	{	71,		79,		89,		100,	112,	126,	141,	158,	178,	200,	224,	251},
	{	282,	316,	355,	398,	447,	501,	562,	631,	708,	794,	891,	1000},
	{	1122,	1259,	1413,	1585,	1778,	1995,	2239,	2512,	2818,	3162,	3548,	3981},
	{	4467,	5012,	5623,	6310,	7079,	7943,	8913,	10000,	11220,	12589,	14125,	15849},
	{	17783,	19953,	22387,	25119,	28184,	31623,	35481,	39811,	44668,	50119,	56234,	65535}};


//============================================================
// Global var
//============================================================

u4Byte	OFDMSwingTable[OFDM_TABLE_SIZE] = {
	0x7f8001fe,	// 0, +6.0dB
	0x788001e2,	// 1, +5.5dB
	0x71c001c7,	// 2, +5.0dB
	0x6b8001ae,	// 3, +4.5dB
	0x65400195,	// 4, +4.0dB
	0x5fc0017f,	// 5, +3.5dB
	0x5a400169,	// 6, +3.0dB
	0x55400155,	// 7, +2.5dB
	0x50800142,	// 8, +2.0dB
	0x4c000130,	// 9, +1.5dB
	0x47c0011f,	// 10, +1.0dB
	0x43c0010f,	// 11, +0.5dB
	0x40000100,	// 12, +0dB
	0x3c8000f2,	// 13, -0.5dB
	0x390000e4,	// 14, -1.0dB
	0x35c000d7,	// 15, -1.5dB
	0x32c000cb,	// 16, -2.0dB
	0x300000c0,	// 17, -2.5dB
	0x2d4000b5,	// 18, -3.0dB
	0x2ac000ab,	// 19, -3.5dB
	0x288000a2,	// 20, -4.0dB
	0x26000098,	// 21, -4.5dB
	0x24000090,	// 22, -5.0dB
	0x22000088,	// 23, -5.5dB
	0x20000080,	// 24, -6.0dB
	0x1e400079,	// 25, -6.5dB
	0x1c800072,	// 26, -7.0dB
	0x1b00006c,	// 27. -7.5dB
	0x19800066,	// 28, -8.0dB
	0x18000060,	// 29, -8.5dB
	0x16c0005b,	// 30, -9.0dB
	0x15800056,	// 31, -9.5dB
	0x14400051,	// 32, -10.0dB
	0x1300004c,	// 33, -10.5dB
	0x12000048,	// 34, -11.0dB
	0x11000044,	// 35, -11.5dB
	0x10000040,	// 36, -12.0dB
};

u1Byte	CCKSwingTable_Ch1_Ch13[CCK_TABLE_SIZE][8] = {
	{0x36, 0x35, 0x2e, 0x25, 0x1c, 0x12, 0x09, 0x04},	// 0, +0dB
	{0x33, 0x32, 0x2b, 0x23, 0x1a, 0x11, 0x08, 0x04},	// 1, -0.5dB
	{0x30, 0x2f, 0x29, 0x21, 0x19, 0x10, 0x08, 0x03},	// 2, -1.0dB
	{0x2d, 0x2d, 0x27, 0x1f, 0x18, 0x0f, 0x08, 0x03},	// 3, -1.5dB
	{0x2b, 0x2a, 0x25, 0x1e, 0x16, 0x0e, 0x07, 0x03},	// 4, -2.0dB 
	{0x28, 0x28, 0x22, 0x1c, 0x15, 0x0d, 0x07, 0x03},	// 5, -2.5dB
	{0x26, 0x25, 0x21, 0x1b, 0x14, 0x0d, 0x06, 0x03},	// 6, -3.0dB
	{0x24, 0x23, 0x1f, 0x19, 0x13, 0x0c, 0x06, 0x03},	// 7, -3.5dB
	{0x22, 0x21, 0x1d, 0x18, 0x11, 0x0b, 0x06, 0x02},	// 8, -4.0dB 
	{0x20, 0x20, 0x1b, 0x16, 0x11, 0x08, 0x05, 0x02},	// 9, -4.5dB
	{0x1f, 0x1e, 0x1a, 0x15, 0x10, 0x0a, 0x05, 0x02},	// 10, -5.0dB 
	{0x1d, 0x1c, 0x18, 0x14, 0x0f, 0x0a, 0x05, 0x02},	// 11, -5.5dB
	{0x1b, 0x1a, 0x17, 0x13, 0x0e, 0x09, 0x04, 0x02},	// 12, -6.0dB <== default
	{0x1a, 0x19, 0x16, 0x12, 0x0d, 0x09, 0x04, 0x02},	// 13, -6.5dB
	{0x18, 0x17, 0x15, 0x11, 0x0c, 0x08, 0x04, 0x02},	// 14, -7.0dB 
	{0x17, 0x16, 0x13, 0x10, 0x0c, 0x08, 0x04, 0x02},	// 15, -7.5dB
	{0x16, 0x15, 0x12, 0x0f, 0x0b, 0x07, 0x04, 0x01},	// 16, -8.0dB 
	{0x14, 0x14, 0x11, 0x0e, 0x0b, 0x07, 0x03, 0x02},	// 17, -8.5dB
	{0x13, 0x13, 0x10, 0x0d, 0x0a, 0x06, 0x03, 0x01},	// 18, -9.0dB 
	{0x12, 0x12, 0x0f, 0x0c, 0x09, 0x06, 0x03, 0x01},	// 19, -9.5dB
	{0x11, 0x11, 0x0f, 0x0c, 0x09, 0x06, 0x03, 0x01},	// 20, -10.0dB
	{0x10, 0x10, 0x0e, 0x0b, 0x08, 0x05, 0x03, 0x01},	// 21, -10.5dB
	{0x0f, 0x0f, 0x0d, 0x0b, 0x08, 0x05, 0x03, 0x01},	// 22, -11.0dB
	{0x0e, 0x0e, 0x0c, 0x0a, 0x08, 0x05, 0x02, 0x01},	// 23, -11.5dB
	{0x0d, 0x0d, 0x0c, 0x0a, 0x07, 0x05, 0x02, 0x01},	// 24, -12.0dB
	{0x0d, 0x0c, 0x0b, 0x09, 0x07, 0x04, 0x02, 0x01},	// 25, -12.5dB
	{0x0c, 0x0c, 0x0a, 0x09, 0x06, 0x04, 0x02, 0x01},	// 26, -13.0dB
	{0x0b, 0x0b, 0x0a, 0x08, 0x06, 0x04, 0x02, 0x01},	// 27, -13.5dB
	{0x0b, 0x0a, 0x09, 0x08, 0x06, 0x04, 0x02, 0x01},	// 28, -14.0dB
	{0x0a, 0x0a, 0x09, 0x07, 0x05, 0x03, 0x02, 0x01},	// 29, -14.5dB
	{0x0a, 0x09, 0x08, 0x07, 0x05, 0x03, 0x02, 0x01},	// 30, -15.0dB
	{0x09, 0x09, 0x08, 0x06, 0x05, 0x03, 0x01, 0x01},	// 31, -15.5dB
	{0x09, 0x08, 0x07, 0x06, 0x04, 0x03, 0x01, 0x01}	// 32, -16.0dB
};


u1Byte	CCKSwingTable_Ch14[CCK_TABLE_SIZE][8] = {
	{0x36, 0x35, 0x2e, 0x1b, 0x00, 0x00, 0x00, 0x00},	// 0, +0dB  
	{0x33, 0x32, 0x2b, 0x19, 0x00, 0x00, 0x00, 0x00},	// 1, -0.5dB 
	{0x30, 0x2f, 0x29, 0x18, 0x00, 0x00, 0x00, 0x00},	// 2, -1.0dB  
	{0x2d, 0x2d, 0x17, 0x17, 0x00, 0x00, 0x00, 0x00},	// 3, -1.5dB
	{0x2b, 0x2a, 0x25, 0x15, 0x00, 0x00, 0x00, 0x00},	// 4, -2.0dB  
	{0x28, 0x28, 0x24, 0x14, 0x00, 0x00, 0x00, 0x00},	// 5, -2.5dB
	{0x26, 0x25, 0x21, 0x13, 0x00, 0x00, 0x00, 0x00},	// 6, -3.0dB  
	{0x24, 0x23, 0x1f, 0x12, 0x00, 0x00, 0x00, 0x00},	// 7, -3.5dB  
	{0x22, 0x21, 0x1d, 0x11, 0x00, 0x00, 0x00, 0x00},	// 8, -4.0dB  
	{0x20, 0x20, 0x1b, 0x10, 0x00, 0x00, 0x00, 0x00},	// 9, -4.5dB
	{0x1f, 0x1e, 0x1a, 0x0f, 0x00, 0x00, 0x00, 0x00},	// 10, -5.0dB  
	{0x1d, 0x1c, 0x18, 0x0e, 0x00, 0x00, 0x00, 0x00},	// 11, -5.5dB
	{0x1b, 0x1a, 0x17, 0x0e, 0x00, 0x00, 0x00, 0x00},	// 12, -6.0dB  <== default
	{0x1a, 0x19, 0x16, 0x0d, 0x00, 0x00, 0x00, 0x00},	// 13, -6.5dB 
	{0x18, 0x17, 0x15, 0x0c, 0x00, 0x00, 0x00, 0x00},	// 14, -7.0dB  
	{0x17, 0x16, 0x13, 0x0b, 0x00, 0x00, 0x00, 0x00},	// 15, -7.5dB
	{0x16, 0x15, 0x12, 0x0b, 0x00, 0x00, 0x00, 0x00},	// 16, -8.0dB  
	{0x14, 0x14, 0x11, 0x0a, 0x00, 0x00, 0x00, 0x00},	// 17, -8.5dB
	{0x13, 0x13, 0x10, 0x0a, 0x00, 0x00, 0x00, 0x00},	// 18, -9.0dB  
	{0x12, 0x12, 0x0f, 0x09, 0x00, 0x00, 0x00, 0x00},	// 19, -9.5dB
	{0x11, 0x11, 0x0f, 0x09, 0x00, 0x00, 0x00, 0x00},	// 20, -10.0dB
	{0x10, 0x10, 0x0e, 0x08, 0x00, 0x00, 0x00, 0x00},	// 21, -10.5dB
	{0x0f, 0x0f, 0x0d, 0x08, 0x00, 0x00, 0x00, 0x00},	// 22, -11.0dB
	{0x0e, 0x0e, 0x0c, 0x07, 0x00, 0x00, 0x00, 0x00},	// 23, -11.5dB
	{0x0d, 0x0d, 0x0c, 0x07, 0x00, 0x00, 0x00, 0x00},	// 24, -12.0dB
	{0x0d, 0x0c, 0x0b, 0x06, 0x00, 0x00, 0x00, 0x00},	// 25, -12.5dB
	{0x0c, 0x0c, 0x0a, 0x06, 0x00, 0x00, 0x00, 0x00},	// 26, -13.0dB
	{0x0b, 0x0b, 0x0a, 0x06, 0x00, 0x00, 0x00, 0x00},	// 27, -13.5dB
	{0x0b, 0x0a, 0x09, 0x05, 0x00, 0x00, 0x00, 0x00},	// 28, -14.0dB
	{0x0a, 0x0a, 0x09, 0x05, 0x00, 0x00, 0x00, 0x00},	// 29, -14.5dB
	{0x0a, 0x09, 0x08, 0x05, 0x00, 0x00, 0x00, 0x00},	// 30, -15.0dB
	{0x09, 0x09, 0x08, 0x05, 0x00, 0x00, 0x00, 0x00},	// 31, -15.5dB
	{0x09, 0x08, 0x07, 0x04, 0x00, 0x00, 0x00, 0x00}	// 32, -16.0dB
};


u4Byte OFDMSwingTable_New[OFDM_TABLE_SIZE] = {
	0x0b40002d, // 0,  -15.0dB	
	0x0c000030, // 1,  -14.5dB
	0x0cc00033, // 2,  -14.0dB
	0x0d800036, // 3,  -13.5dB
	0x0e400039, // 4,  -13.0dB    
	0x0f00003c, // 5,  -12.5dB
	0x10000040, // 6,  -12.0dB
	0x11000044, // 7,  -11.5dB
	0x12000048, // 8,  -11.0dB
	0x1300004c, // 9,  -10.5dB
	0x14400051, // 10, -10.0dB
	0x15800056, // 11, -9.5dB
	0x16c0005b, // 12, -9.0dB
	0x18000060, // 13, -8.5dB
	0x19800066, // 14, -8.0dB
	0x1b00006c, // 15, -7.5dB
	0x1c800072, // 16, -7.0dB
	0x1e400079, // 17, -6.5dB
	0x20000080, // 18, -6.0dB
	0x22000088, // 19, -5.5dB
	0x24000090, // 20, -5.0dB
	0x26000098, // 21, -4.5dB
	0x288000a2, // 22, -4.0dB
	0x2ac000ab, // 23, -3.5dB
	0x2d4000b5, // 24, -3.0dB
	0x300000c0, // 25, -2.5dB
	0x32c000cb, // 26, -2.0dB
	0x35c000d7, // 27, -1.5dB
	0x390000e4, // 28, -1.0dB
	0x3c8000f2, // 29, -0.5dB
	0x40000100, // 30, +0dB
	0x43c0010f, // 31, +0.5dB
	0x47c0011f, // 32, +1.0dB
	0x4c000130, // 33, +1.5dB
	0x50800142, // 34, +2.0dB
	0x55400155, // 35, +2.5dB
	0x5a400169, // 36, +3.0dB
	0x5fc0017f, // 37, +3.5dB
	0x65400195, // 38, +4.0dB
	0x6b8001ae, // 39, +4.5dB
	0x71c001c7, // 40, +5.0dB
	0x788001e2, // 41, +5.5dB
	0x7f8001fe  // 42, +6.0dB
};               


u1Byte CCKSwingTable_Ch1_Ch13_New[CCK_TABLE_SIZE][8] = {
	{0x09, 0x08, 0x07, 0x06, 0x04, 0x03, 0x01, 0x01},	//  0, -16.0dB
	{0x09, 0x09, 0x08, 0x06, 0x05, 0x03, 0x01, 0x01},	//  1, -15.5dB
	{0x0a, 0x09, 0x08, 0x07, 0x05, 0x03, 0x02, 0x01},	//  2, -15.0dB
	{0x0a, 0x0a, 0x09, 0x07, 0x05, 0x03, 0x02, 0x01},	//  3, -14.5dB
	{0x0b, 0x0a, 0x09, 0x08, 0x06, 0x04, 0x02, 0x01},	//  4, -14.0dB
	{0x0b, 0x0b, 0x0a, 0x08, 0x06, 0x04, 0x02, 0x01},	//  5, -13.5dB
	{0x0c, 0x0c, 0x0a, 0x09, 0x06, 0x04, 0x02, 0x01},	//  6, -13.0dB
	{0x0d, 0x0c, 0x0b, 0x09, 0x07, 0x04, 0x02, 0x01},	//  7, -12.5dB
	{0x0d, 0x0d, 0x0c, 0x0a, 0x07, 0x05, 0x02, 0x01},	//  8, -12.0dB
	{0x0e, 0x0e, 0x0c, 0x0a, 0x08, 0x05, 0x02, 0x01},	//  9, -11.5dB
	{0x0f, 0x0f, 0x0d, 0x0b, 0x08, 0x05, 0x03, 0x01},	// 10, -11.0dB
	{0x10, 0x10, 0x0e, 0x0b, 0x08, 0x05, 0x03, 0x01},	// 11, -10.5dB
	{0x11, 0x11, 0x0f, 0x0c, 0x09, 0x06, 0x03, 0x01},	// 12, -10.0dB
	{0x12, 0x12, 0x0f, 0x0c, 0x09, 0x06, 0x03, 0x01},	// 13, -9.5dB
	{0x13, 0x13, 0x10, 0x0d, 0x0a, 0x06, 0x03, 0x01},	// 14, -9.0dB 
	{0x14, 0x14, 0x11, 0x0e, 0x0b, 0x07, 0x03, 0x02},	// 15, -8.5dB
	{0x16, 0x15, 0x12, 0x0f, 0x0b, 0x07, 0x04, 0x01},	// 16, -8.0dB 
	{0x17, 0x16, 0x13, 0x10, 0x0c, 0x08, 0x04, 0x02},	// 17, -7.5dB
	{0x18, 0x17, 0x15, 0x11, 0x0c, 0x08, 0x04, 0x02},	// 18, -7.0dB 
	{0x1a, 0x19, 0x16, 0x12, 0x0d, 0x09, 0x04, 0x02},	// 19, -6.5dB
    {0x1b, 0x1a, 0x17, 0x13, 0x0e, 0x09, 0x04, 0x02},	// 20, -6.0dB 
	{0x1d, 0x1c, 0x18, 0x14, 0x0f, 0x0a, 0x05, 0x02},	// 21, -5.5dB
	{0x1f, 0x1e, 0x1a, 0x15, 0x10, 0x0a, 0x05, 0x02},	// 22, -5.0dB 
	{0x20, 0x20, 0x1b, 0x16, 0x11, 0x08, 0x05, 0x02},	// 23, -4.5dB
	{0x22, 0x21, 0x1d, 0x18, 0x11, 0x0b, 0x06, 0x02},	// 24, -4.0dB 
	{0x24, 0x23, 0x1f, 0x19, 0x13, 0x0c, 0x06, 0x03},	// 25, -3.5dB
	{0x26, 0x25, 0x21, 0x1b, 0x14, 0x0d, 0x06, 0x03},	// 26, -3.0dB
	{0x28, 0x28, 0x22, 0x1c, 0x15, 0x0d, 0x07, 0x03},	// 27, -2.5dB
	{0x2b, 0x2a, 0x25, 0x1e, 0x16, 0x0e, 0x07, 0x03},	// 28, -2.0dB 
	{0x2d, 0x2d, 0x27, 0x1f, 0x18, 0x0f, 0x08, 0x03},	// 29, -1.5dB
	{0x30, 0x2f, 0x29, 0x21, 0x19, 0x10, 0x08, 0x03},	// 30, -1.0dB
	{0x33, 0x32, 0x2b, 0x23, 0x1a, 0x11, 0x08, 0x04},	// 31, -0.5dB
	{0x36, 0x35, 0x2e, 0x25, 0x1c, 0x12, 0x09, 0x04} 	// 32, +0dB
};                                                                  


u1Byte CCKSwingTable_Ch14_New[CCK_TABLE_SIZE][8]= {
	{0x09, 0x08, 0x07, 0x04, 0x00, 0x00, 0x00, 0x00},	//  0, -16.0dB
	{0x09, 0x09, 0x08, 0x05, 0x00, 0x00, 0x00, 0x00},	//  1, -15.5dB
	{0x0a, 0x09, 0x08, 0x05, 0x00, 0x00, 0x00, 0x00},	//  2, -15.0dB
	{0x0a, 0x0a, 0x09, 0x05, 0x00, 0x00, 0x00, 0x00},	//  3, -14.5dB
	{0x0b, 0x0a, 0x09, 0x05, 0x00, 0x00, 0x00, 0x00},	//  4, -14.0dB
	{0x0b, 0x0b, 0x0a, 0x06, 0x00, 0x00, 0x00, 0x00},	//  5, -13.5dB
	{0x0c, 0x0c, 0x0a, 0x06, 0x00, 0x00, 0x00, 0x00},	//  6, -13.0dB
	{0x0d, 0x0c, 0x0b, 0x06, 0x00, 0x00, 0x00, 0x00},	//  7, -12.5dB
	{0x0d, 0x0d, 0x0c, 0x07, 0x00, 0x00, 0x00, 0x00},	//  8, -12.0dB
	{0x0e, 0x0e, 0x0c, 0x07, 0x00, 0x00, 0x00, 0x00},	//  9, -11.5dB
	{0x0f, 0x0f, 0x0d, 0x08, 0x00, 0x00, 0x00, 0x00},	// 10, -11.0dB
	{0x10, 0x10, 0x0e, 0x08, 0x00, 0x00, 0x00, 0x00},	// 11, -10.5dB
	{0x11, 0x11, 0x0f, 0x09, 0x00, 0x00, 0x00, 0x00},	// 12, -10.0dB
	{0x12, 0x12, 0x0f, 0x09, 0x00, 0x00, 0x00, 0x00},	// 13, -9.5dB
	{0x13, 0x13, 0x10, 0x0a, 0x00, 0x00, 0x00, 0x00},	// 14, -9.0dB  
	{0x14, 0x14, 0x11, 0x0a, 0x00, 0x00, 0x00, 0x00},	// 15, -8.5dB
	{0x16, 0x15, 0x12, 0x0b, 0x00, 0x00, 0x00, 0x00},	// 16, -8.0dB  
	{0x17, 0x16, 0x13, 0x0b, 0x00, 0x00, 0x00, 0x00},	// 17, -7.5dB
	{0x18, 0x17, 0x15, 0x0c, 0x00, 0x00, 0x00, 0x00},	// 18, -7.0dB  
	{0x1a, 0x19, 0x16, 0x0d, 0x00, 0x00, 0x00, 0x00},	// 19, -6.5dB 
	{0x1b, 0x1a, 0x17, 0x0e, 0x00, 0x00, 0x00, 0x00},	// 20, -6.0dB  
	{0x1d, 0x1c, 0x18, 0x0e, 0x00, 0x00, 0x00, 0x00},	// 21, -5.5dB
	{0x1f, 0x1e, 0x1a, 0x0f, 0x00, 0x00, 0x00, 0x00},	// 22, -5.0dB  
	{0x20, 0x20, 0x1b, 0x10, 0x00, 0x00, 0x00, 0x00},	// 23, -4.5dB
	{0x22, 0x21, 0x1d, 0x11, 0x00, 0x00, 0x00, 0x00},	// 24, -4.0dB  
	{0x24, 0x23, 0x1f, 0x12, 0x00, 0x00, 0x00, 0x00},	// 25, -3.5dB  
	{0x26, 0x25, 0x21, 0x13, 0x00, 0x00, 0x00, 0x00},	// 26, -3.0dB  
	{0x28, 0x28, 0x24, 0x14, 0x00, 0x00, 0x00, 0x00},	// 27, -2.5dB
	{0x2b, 0x2a, 0x25, 0x15, 0x00, 0x00, 0x00, 0x00},	// 28, -2.0dB  
	{0x2d, 0x2d, 0x17, 0x17, 0x00, 0x00, 0x00, 0x00},	// 29, -1.5dB
	{0x30, 0x2f, 0x29, 0x18, 0x00, 0x00, 0x00, 0x00},	// 30, -1.0dB  
	{0x33, 0x32, 0x2b, 0x19, 0x00, 0x00, 0x00, 0x00},	// 31, -0.5dB 
	{0x36, 0x35, 0x2e, 0x1b, 0x00, 0x00, 0x00, 0x00} 	// 32, +0dB	
};

u4Byte TxScalingTable_Jaguar[TXSCALE_TABLE_SIZE] =
{
	0x081, // 0,  -12.0dB
	0x088, // 1,  -11.5dB
	0x090, // 2,  -11.0dB
	0x099, // 3,  -10.5dB
	0x0A2, // 4,  -10.0dB
	0x0AC, // 5,  -9.5dB
	0x0B6, // 6,  -9.0dB
	0x0C0, // 7,  -8.5dB
	0x0CC, // 8,  -8.0dB
	0x0D8, // 9,  -7.5dB
	0x0E5, // 10, -7.0dB
	0x0F2, // 11, -6.5dB
	0x101, // 12, -6.0dB
	0x110, // 13, -5.5dB
	0x120, // 14, -5.0dB
	0x131, // 15, -4.5dB
	0x143, // 16, -4.0dB
	0x156, // 17, -3.5dB
	0x16A, // 18, -3.0dB
	0x180, // 19, -2.5dB
	0x197, // 20, -2.0dB
	0x1AF, // 21, -1.5dB
	0x1C8, // 22, -1.0dB
	0x1E3, // 23, -0.5dB
	0x200, // 24, +0  dB
	0x21E, // 25, +0.5dB
	0x23E, // 26, +1.0dB
	0x261, // 27, +1.5dB
	0x285, // 28, +2.0dB
	0x2AB, // 29, +2.5dB
	0x2D3, // 30, +3.0dB
	0x2FE, // 31, +3.5dB
	0x32B, // 32, +4.0dB
	0x35C, // 33, +4.5dB
	0x38E, // 34, +5.0dB
	0x3C4, // 35, +5.5dB
	0x3FE  // 36, +6.0dB	
};

//============================================================
// Local Function predefine.
//============================================================

//START------------COMMON INFO RELATED---------------//
void
odm_CommonInfoSelfInit(
	IN		PDM_ODM_T		pDM_Odm
	);

void
odm_CommonInfoSelfUpdate(
	IN		PDM_ODM_T		pDM_Odm
	);

void
odm_CmnInfoInit_Debug(
	IN		PDM_ODM_T		pDM_Odm
	);

void
odm_CmnInfoHook_Debug(
	IN		PDM_ODM_T		pDM_Odm
	);

void
odm_CmnInfoUpdate_Debug(
	IN		PDM_ODM_T		pDM_Odm
	);
void
odm_BasicDbgMessage
(
	IN		PDM_ODM_T		pDM_Odm
	);

//END------------COMMON INFO RELATED---------------//

//START---------------DIG---------------------------//

//Remove by Yuchen

//END---------------DIG---------------------------//

//START-------BB POWER SAVE-----------------------//
//Remove BB power Saving by YuChen
//END---------BB POWER SAVE-----------------------//

void
odm_RefreshRateAdaptiveMaskMP(
	IN		PDM_ODM_T		pDM_Odm
	);

void
odm_RefreshRateAdaptiveMaskCE(
	IN		PDM_ODM_T		pDM_Odm
	);

void
odm_RefreshRateAdaptiveMaskAPADSL(
	IN		PDM_ODM_T		pDM_Odm
	);

//Remove by YuChen

void
odm_RSSIMonitorInit(
	IN	PDM_ODM_T	pDM_Odm
	);

void
odm_RSSIMonitorCheckMP(
	IN	PDM_ODM_T	pDM_Odm
	);

void 
odm_RSSIMonitorCheckCE(
	IN		PDM_ODM_T		pDM_Odm
	);
void 
odm_RSSIMonitorCheckAP(
	IN		PDM_ODM_T		pDM_Odm
	);



void
odm_RSSIMonitorCheck(
	IN		PDM_ODM_T		pDM_Odm
	);

void
odm_SwAntDetectInit(
	IN 		PDM_ODM_T 		pDM_Odm
	);

void
odm_AntennaDiversityInit(
	IN		PDM_ODM_T		pDM_Odm
	);

void
odm_AntennaDiversity(
	IN		PDM_ODM_T		pDM_Odm
	);


void odm_SwAntDivChkAntSwitchCallback(void *FunctionContext);



void
odm_GlobalAdapterCheck(
	IN		void
	);

void
odm_RefreshBasicRateMask(
	IN		PDM_ODM_T		pDM_Odm	
	);

void
odm_RefreshRateAdaptiveMask(
	IN		PDM_ODM_T		pDM_Odm
	);

void
ODM_TXPowerTrackingCheck(
	IN		PDM_ODM_T		pDM_Odm
	);

void
odm_TXPowerTrackingCheckAP(
	IN		PDM_ODM_T		pDM_Odm
	);

void
odm_RateAdaptiveMaskInit(
	IN	PDM_ODM_T	pDM_Odm
	);

void
odm_TXPowerTrackingThermalMeterInit(
	IN	PDM_ODM_T	pDM_Odm
	);


void
odm_TXPowerTrackingInit(
	IN	PDM_ODM_T	pDM_Odm
	);

void
odm_TXPowerTrackingCheckMP(
	IN	PDM_ODM_T	pDM_Odm
	);


void
odm_TXPowerTrackingCheckCE(
	IN	PDM_ODM_T	pDM_Odm
	);

//Remove Edca by Yu Chen


#define 	RxDefaultAnt1		0x65a9
#define	RxDefaultAnt2		0x569a

void
odm_InitHybridAntDiv(
	IN PDM_ODM_T	pDM_Odm 
	);

bool
odm_StaDefAntSel(
	IN PDM_ODM_T	pDM_Odm,
	IN u4Byte		OFDM_Ant1_Cnt,
	IN u4Byte		OFDM_Ant2_Cnt,
	IN u4Byte		CCK_Ant1_Cnt,
	IN u4Byte		CCK_Ant2_Cnt,
	OUT u1Byte		*pDefAnt 
	);

void
odm_SetRxIdleAnt(
	IN	PDM_ODM_T	pDM_Odm,
	IN	u1Byte	Ant,
	IN   bool   bDualPath                     
);



void
odm_HwAntDiv(
	IN	PDM_ODM_T	pDM_Odm
);


//============================================================
//3 Export Interface
//============================================================

//
// 2011/09/21 MH Add to describe different team necessary resource allocate??
//
void
ODM_DMInit(
	IN		PDM_ODM_T		pDM_Odm
	)
{

	odm_CommonInfoSelfInit(pDM_Odm);
	odm_CmnInfoInit_Debug(pDM_Odm);
	odm_DIGInit(pDM_Odm);
	odm_NHMCounterStatisticsInit(pDM_Odm);
	odm_AdaptivityInit(pDM_Odm);
	odm_RateAdaptiveMaskInit(pDM_Odm);
	ODM_CfoTrackingInit(pDM_Odm);
	ODM_EdcaTurboInit(pDM_Odm);
	odm_RSSIMonitorInit(pDM_Odm);
	odm_TXPowerTrackingInit(pDM_Odm);
	odm_AntennaDiversityInit(pDM_Odm);

	ODM_ClearTxPowerTrackingState(pDM_Odm);

	if ( *(pDM_Odm->mp_mode) != 1)
	odm_PathDiversityInit(pDM_Odm);

	odm_DynamicBBPowerSavingInit(pDM_Odm);
	odm_DynamicTxPowerInit(pDM_Odm);

	odm_SwAntDetectInit(pDM_Odm);
}

void
ODM_DMReset(
	IN		PDM_ODM_T		pDM_Odm
	)
{
}

//
// 2011/09/20 MH This is the entry pointer for all team to execute HW out source DM.
// You can not add any dummy function here, be care, you can only use DM structure
// to perform any new ODM_DM.
//
void
ODM_DMWatchdog(
	IN		PDM_ODM_T		pDM_Odm
	)
{	
	odm_CommonInfoSelfUpdate(pDM_Odm);
	odm_BasicDbgMessage(pDM_Odm);
	odm_FalseAlarmCounterStatistics(pDM_Odm);
	odm_NHMCounterStatistics(pDM_Odm);
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_DIG(): RSSI=0x%x\n",pDM_Odm->RSSI_Min));

	odm_RSSIMonitorCheck(pDM_Odm);

	//For CE Platform(SPRD or Tablet)
	//8723A or 8189ES platform
	//NeilChen--2012--08--24--
	//Fix Leave LPS issue
	if( 	(adapter_to_pwrctl(pDM_Odm->Adapter)->pwr_mode != PS_MODE_ACTIVE) // in LPS mode
		//&&( 			
		//	(pDM_Odm->SupportICType & (ODM_RTL8723A ) )||
		//   	(pDM_Odm->SupportICType & (ODM_RTL8188E) &&((pDM_Odm->SupportInterface  == ODM_ITRF_SDIO)) ) 
	  	//)	
	)
	{
			ODM_RT_TRACE(pDM_Odm,ODM_COMP_DIG, ODM_DBG_LOUD, ("----Step1: odm_DIG is in LPS mode\n"));				
			ODM_RT_TRACE(pDM_Odm,ODM_COMP_DIG, ODM_DBG_LOUD, ("---Step2: 8723AS is in LPS mode\n"));
			odm_DIGbyRSSI_LPS(pDM_Odm);
	}		
	else				
	{
		odm_DIG(pDM_Odm);
	}

	{
		pDIG_T	pDM_DigTable = &pDM_Odm->DM_DigTable;
		odm_Adaptivity(pDM_Odm, pDM_DigTable->CurIGValue);
	}
	odm_CCKPacketDetectionThresh(pDM_Odm);

	if(*(pDM_Odm->pbPowerSaving)==true)
		return;

	
	odm_RefreshRateAdaptiveMask(pDM_Odm);
	odm_RefreshBasicRateMask(pDM_Odm);
	odm_EdcaTurboCheck(pDM_Odm);
	odm_PathDiversity(pDM_Odm);
	ODM_CfoTracking(pDM_Odm);
	odm_AntennaDiversity(pDM_Odm);

	ODM_TXPowerTrackingCheck(pDM_Odm);

	//odm_EdcaTurboCheck(pDM_Odm);

	//2010.05.30 LukeLee: For CE platform, files in IC subfolders may not be included to be compiled,
	// so compile flags must be left here to prevent from compile errors
	pDM_Odm->PhyDbgInfo.NumQryBeaconPkt = 0;
}


//
// Init /.. Fixed HW value. Only init time.
//
void
ODM_CmnInfoInit(
	IN		PDM_ODM_T		pDM_Odm,
	IN		ODM_CMNINFO_E	CmnInfo,
	IN		u4Byte			Value	
	)
{
	//
	// This section is used for init value
	//
	switch	(CmnInfo)
	{
		//
		// Fixed ODM value.
		//
		case	ODM_CMNINFO_ABILITY:
			pDM_Odm->SupportAbility = (u4Byte)Value;
			break;

		case	ODM_CMNINFO_RF_TYPE:
			pDM_Odm->RFType = (u1Byte)Value;
			break;

		case	ODM_CMNINFO_PLATFORM:
			pDM_Odm->SupportPlatform = (u1Byte)Value;
			break;

		case	ODM_CMNINFO_INTERFACE:
			pDM_Odm->SupportInterface = (u1Byte)Value;
			break;

		case	ODM_CMNINFO_MP_TEST_CHIP:
			pDM_Odm->bIsMPChip= (u1Byte)Value;
			break;
            
		case	ODM_CMNINFO_IC_TYPE:
			pDM_Odm->SupportICType = Value;
			break;

		case	ODM_CMNINFO_CUT_VER:
			pDM_Odm->CutVersion = (u1Byte)Value;
			break;

		case	ODM_CMNINFO_FAB_VER:
			pDM_Odm->FabVersion = (u1Byte)Value;
			break;

		case	ODM_CMNINFO_RFE_TYPE:
			pDM_Odm->RFEType = (u1Byte)Value;
			break;

		case    ODM_CMNINFO_RF_ANTENNA_TYPE:
			pDM_Odm->AntDivType= (u1Byte)Value;
			break;

		case	ODM_CMNINFO_BOARD_TYPE:
			pDM_Odm->BoardType = (u1Byte)Value;
			break;

		case	ODM_CMNINFO_PACKAGE_TYPE:
			pDM_Odm->PackageType = (u1Byte)Value;
			break;

		case	ODM_CMNINFO_EXT_LNA:
			pDM_Odm->ExtLNA = (u1Byte)Value;
			break;

		case	ODM_CMNINFO_5G_EXT_LNA:
			pDM_Odm->ExtLNA5G = (u1Byte)Value;
			break;

		case	ODM_CMNINFO_EXT_PA:
			pDM_Odm->ExtPA = (u1Byte)Value;
			break;

		case	ODM_CMNINFO_5G_EXT_PA:
			pDM_Odm->ExtPA5G = (u1Byte)Value;
			break;

		case	ODM_CMNINFO_GPA:
			pDM_Odm->TypeGPA= (ODM_TYPE_GPA_E)Value;
			break;
		case	ODM_CMNINFO_APA:
			pDM_Odm->TypeAPA= (ODM_TYPE_APA_E)Value;
			break;
		case	ODM_CMNINFO_GLNA:
			pDM_Odm->TypeGLNA= (ODM_TYPE_GLNA_E)Value;
			break;
		case	ODM_CMNINFO_ALNA:
			pDM_Odm->TypeALNA= (ODM_TYPE_ALNA_E)Value;
			break;

		case	ODM_CMNINFO_EXT_TRSW:
			pDM_Odm->ExtTRSW = (u1Byte)Value;
			break;
		case 	ODM_CMNINFO_PATCH_ID:
			pDM_Odm->PatchID = (u1Byte)Value;
			break;
		case 	ODM_CMNINFO_BINHCT_TEST:
			pDM_Odm->bInHctTest = (bool)Value;
			break;
		case 	ODM_CMNINFO_BWIFI_TEST:
			pDM_Odm->bWIFITest = (bool)Value;
			break;	

		case	ODM_CMNINFO_SMART_CONCURRENT:
			pDM_Odm->bDualMacSmartConcurrent = (bool )Value;
			break;
		
		//To remove the compiler warning, must add an empty default statement to handle the other values.	
		default:
			//do nothing
			break;	
		
	}

}


void
ODM_CmnInfoHook(
	IN		PDM_ODM_T		pDM_Odm,
	IN		ODM_CMNINFO_E	CmnInfo,
	IN		void *			pValue	
	)
{
	//
	// Hook call by reference pointer.
	//
	switch	(CmnInfo)
	{
		//
		// Dynamic call by reference pointer.
		//
		case	ODM_CMNINFO_MAC_PHY_MODE:
			pDM_Odm->pMacPhyMode = (u1Byte *)pValue;
			break;
		
		case	ODM_CMNINFO_TX_UNI:
			pDM_Odm->pNumTxBytesUnicast = (u8Byte *)pValue;
			break;

		case	ODM_CMNINFO_RX_UNI:
			pDM_Odm->pNumRxBytesUnicast = (u8Byte *)pValue;
			break;

		case	ODM_CMNINFO_WM_MODE:
			pDM_Odm->pWirelessMode = (u1Byte *)pValue;
			break;

		case	ODM_CMNINFO_BAND:
			pDM_Odm->pBandType = (u1Byte *)pValue;
			break;

		case	ODM_CMNINFO_SEC_CHNL_OFFSET:
			pDM_Odm->pSecChOffset = (u1Byte *)pValue;
			break;

		case	ODM_CMNINFO_SEC_MODE:
			pDM_Odm->pSecurity = (u1Byte *)pValue;
			break;

		case	ODM_CMNINFO_BW:
			pDM_Odm->pBandWidth = (u1Byte *)pValue;
			break;

		case	ODM_CMNINFO_CHNL:
			pDM_Odm->pChannel = (u1Byte *)pValue;
			break;
		
		case	ODM_CMNINFO_DMSP_GET_VALUE:
			pDM_Odm->pbGetValueFromOtherMac = (bool *)pValue;
			break;

		case	ODM_CMNINFO_BUDDY_ADAPTOR:
			pDM_Odm->pBuddyAdapter = (PADAPTER *)pValue;
			break;

		case	ODM_CMNINFO_DMSP_IS_MASTER:
			pDM_Odm->pbMasterOfDMSP = (bool *)pValue;
			break;

		case	ODM_CMNINFO_SCAN:
			pDM_Odm->pbScanInProcess = (bool *)pValue;
			break;

		case	ODM_CMNINFO_POWER_SAVING:
			pDM_Odm->pbPowerSaving = (bool *)pValue;
			break;

		case	ODM_CMNINFO_ONE_PATH_CCA:
			pDM_Odm->pOnePathCCA = (u1Byte *)pValue;
			break;

		case	ODM_CMNINFO_DRV_STOP:
			pDM_Odm->pbDriverStopped =  (bool *)pValue;
			break;

		case	ODM_CMNINFO_PNP_IN:
			pDM_Odm->pbDriverIsGoingToPnpSetPowerSleep =  (bool *)pValue;
			break;

		case	ODM_CMNINFO_INIT_ON:
			pDM_Odm->pinit_adpt_in_progress =  (bool *)pValue;
			break;

		case	ODM_CMNINFO_ANT_TEST:
			pDM_Odm->pAntennaTest =  (u1Byte *)pValue;
			break;

		case	ODM_CMNINFO_NET_CLOSED:
			pDM_Odm->pbNet_closed = (bool *)pValue;
			break;

		case 	ODM_CMNINFO_FORCED_RATE:
			pDM_Odm->pForcedDataRate = (pu2Byte)pValue;
			break;

		case  ODM_CMNINFO_FORCED_IGI_LB:
			pDM_Odm->pu1ForcedIgiLb = (u1Byte *)pValue;
			break;

		case	ODM_CMNINFO_MP_MODE:
			pDM_Odm->mp_mode = (u1Byte *)pValue;
			break;

		//case	ODM_CMNINFO_RTSTA_AID:
		//	pDM_Odm->pAidMap =  (u1Byte *)pValue;
		//	break;

		//case	ODM_CMNINFO_BT_COEXIST:
		//	pDM_Odm->BTCoexist = (bool *)pValue;		

		//case	ODM_CMNINFO_STA_STATUS:
			//pDM_Odm->pODM_StaInfo[] = (PSTA_INFO_T)pValue;
			//break;

		//case	ODM_CMNINFO_PHY_STATUS:
		//	pDM_Odm->pPhyInfo = (ODM_PHY_INFO *)pValue;
		//	break;

		//case	ODM_CMNINFO_MAC_STATUS:
		//	pDM_Odm->pMacInfo = (ODM_MAC_INFO *)pValue;
		//	break;
		//To remove the compiler warning, must add an empty default statement to handle the other values.				
		default:
			//do nothing
			break;

	}

}


void
ODM_CmnInfoPtrArrayHook(
	IN		PDM_ODM_T		pDM_Odm,
	IN		ODM_CMNINFO_E	CmnInfo,
	IN		u2Byte			Index,
	IN		void *			pValue	
	)
{
	//
	// Hook call by reference pointer.
	//
	switch	(CmnInfo)
	{
		//
		// Dynamic call by reference pointer.
		//		
		case	ODM_CMNINFO_STA_STATUS:
			pDM_Odm->pODM_StaInfo[Index] = (PSTA_INFO_T)pValue;
			break;		
		//To remove the compiler warning, must add an empty default statement to handle the other values.				
		default:
			//do nothing
			break;
	}
	
}


//
// Update Band/CHannel/.. The values are dynamic but non-per-packet.
//
void
ODM_CmnInfoUpdate(
	IN		PDM_ODM_T		pDM_Odm,
	IN		u4Byte			CmnInfo,
	IN		u8Byte			Value	
	)
{
	//
	// This init variable may be changed in run time.
	//
	switch	(CmnInfo)
	{
		case ODM_CMNINFO_LINK_IN_PROGRESS:
			pDM_Odm->bLinkInProcess = (bool)Value;
			break;
		
		case	ODM_CMNINFO_ABILITY:
			pDM_Odm->SupportAbility = (u4Byte)Value;
			break;

		case	ODM_CMNINFO_RF_TYPE:
			pDM_Odm->RFType = (u1Byte)Value;
			break;

		case	ODM_CMNINFO_WIFI_DIRECT:
			pDM_Odm->bWIFI_Direct = (bool)Value;
			break;

		case	ODM_CMNINFO_WIFI_DISPLAY:
			pDM_Odm->bWIFI_Display = (bool)Value;
			break;

		case	ODM_CMNINFO_LINK:
			pDM_Odm->bLinked = (bool)Value;
			break;
			
		case	ODM_CMNINFO_STATION_STATE:
			pDM_Odm->bsta_state = (bool)Value;
			break;
			
		case	ODM_CMNINFO_RSSI_MIN:
			pDM_Odm->RSSI_Min= (u1Byte)Value;
			break;

		case	ODM_CMNINFO_DBG_COMP:
			pDM_Odm->DebugComponents = Value;
			break;

		case	ODM_CMNINFO_DBG_LEVEL:
			pDM_Odm->DebugLevel = (u4Byte)Value;
			break;
		case	ODM_CMNINFO_RA_THRESHOLD_HIGH:
			pDM_Odm->RateAdaptive.HighRSSIThresh = (u1Byte)Value;
			break;

		case	ODM_CMNINFO_RA_THRESHOLD_LOW:
			pDM_Odm->RateAdaptive.LowRSSIThresh = (u1Byte)Value;
			break;
		// The following is for BT HS mode and BT coexist mechanism.
		case ODM_CMNINFO_BT_ENABLED:
			pDM_Odm->bBtEnabled = (bool)Value;
			break;
			
		case ODM_CMNINFO_BT_HS_CONNECT_PROCESS:
			pDM_Odm->bBtConnectProcess = (bool)Value;
			break;
		
		case ODM_CMNINFO_BT_HS_RSSI:
			pDM_Odm->btHsRssi = (u1Byte)Value;
			break;
			
		case	ODM_CMNINFO_BT_OPERATION:
			pDM_Odm->bBtHsOperation = (bool)Value;
			break;

		case	ODM_CMNINFO_BT_LIMITED_DIG:
			pDM_Odm->bBtLimitedDig = (bool)Value;
			break;	

		case	ODM_CMNINFO_BT_DISABLE_EDCA:
			pDM_Odm->bBtDisableEdcaTurbo = (bool)Value;
			break;
			
/*
		case	ODM_CMNINFO_OP_MODE:
			pDM_Odm->OPMode = (u1Byte)Value;
			break;

		case	ODM_CMNINFO_WM_MODE:
			pDM_Odm->WirelessMode = (u1Byte)Value;
			break;

		case	ODM_CMNINFO_BAND:
			pDM_Odm->BandType = (u1Byte)Value;
			break;

		case	ODM_CMNINFO_SEC_CHNL_OFFSET:
			pDM_Odm->SecChOffset = (u1Byte)Value;
			break;

		case	ODM_CMNINFO_SEC_MODE:
			pDM_Odm->Security = (u1Byte)Value;
			break;

		case	ODM_CMNINFO_BW:
			pDM_Odm->BandWidth = (u1Byte)Value;
			break;

		case	ODM_CMNINFO_CHNL:
			pDM_Odm->Channel = (u1Byte)Value;
			break;			
*/	
                default:
			//do nothing
			break;
	}

	
}

void
odm_CommonInfoSelfInit(
	IN		PDM_ODM_T		pDM_Odm
	)
{
	pFAT_T			pDM_FatTable = &pDM_Odm->DM_FatTable;
	pDM_Odm->bCckHighPower = (bool) ODM_GetBBReg(pDM_Odm, ODM_REG(CCK_RPT_FORMAT,pDM_Odm), ODM_BIT(CCK_RPT_FORMAT,pDM_Odm));		
	pDM_Odm->RFPathRxEnable = (u1Byte) ODM_GetBBReg(pDM_Odm, ODM_REG(BB_RX_PATH,pDM_Odm), ODM_BIT(BB_RX_PATH,pDM_Odm));

	ODM_InitDebugSetting(pDM_Odm);

	pDM_Odm->TxRate = 0xFF;
}

void
odm_CommonInfoSelfUpdate(
	IN		PDM_ODM_T		pDM_Odm
	)
{
	u1Byte	EntryCnt=0;
	u1Byte	i;
	PSTA_INFO_T   	pEntry;

	if(*(pDM_Odm->pBandWidth) == ODM_BW40M)
	{
		if(*(pDM_Odm->pSecChOffset) == 1)
			pDM_Odm->ControlChannel = *(pDM_Odm->pChannel) -2;
		else if(*(pDM_Odm->pSecChOffset) == 2)
			pDM_Odm->ControlChannel = *(pDM_Odm->pChannel) +2;
	}
	else
		pDM_Odm->ControlChannel = *(pDM_Odm->pChannel);

	for (i=0; i<ODM_ASSOCIATE_ENTRY_NUM; i++)
	{
		pEntry = pDM_Odm->pODM_StaInfo[i];
		if(IS_STA_VALID(pEntry))
			EntryCnt++;
	}
	if(EntryCnt == 1)
		pDM_Odm->bOneEntryOnly = true;
	else
		pDM_Odm->bOneEntryOnly = false;
}

void
odm_CmnInfoInit_Debug(
	IN		PDM_ODM_T		pDM_Odm
	)
{
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_COMMON, ODM_DBG_LOUD, ("odm_CmnInfoInit_Debug==>\n"));
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_COMMON, ODM_DBG_LOUD, ("SupportPlatform=%d\n",pDM_Odm->SupportPlatform) );
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_COMMON, ODM_DBG_LOUD, ("SupportAbility=0x%x\n",pDM_Odm->SupportAbility) );
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_COMMON, ODM_DBG_LOUD, ("SupportInterface=%d\n",pDM_Odm->SupportInterface) );
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_COMMON, ODM_DBG_LOUD, ("SupportICType=0x%x\n",pDM_Odm->SupportICType) );
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_COMMON, ODM_DBG_LOUD, ("CutVersion=%d\n",pDM_Odm->CutVersion) );
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_COMMON, ODM_DBG_LOUD, ("FabVersion=%d\n",pDM_Odm->FabVersion) );
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_COMMON, ODM_DBG_LOUD, ("RFType=%d\n",pDM_Odm->RFType) );
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_COMMON, ODM_DBG_LOUD, ("BoardType=%d\n",pDM_Odm->BoardType) );
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_COMMON, ODM_DBG_LOUD, ("ExtLNA=%d\n",pDM_Odm->ExtLNA) );
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_COMMON, ODM_DBG_LOUD, ("ExtPA=%d\n",pDM_Odm->ExtPA) );
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_COMMON, ODM_DBG_LOUD, ("ExtTRSW=%d\n",pDM_Odm->ExtTRSW) );
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_COMMON, ODM_DBG_LOUD, ("PatchID=%d\n",pDM_Odm->PatchID) );
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_COMMON, ODM_DBG_LOUD, ("bInHctTest=%d\n",pDM_Odm->bInHctTest) );
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_COMMON, ODM_DBG_LOUD, ("bWIFITest=%d\n",pDM_Odm->bWIFITest) );
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_COMMON, ODM_DBG_LOUD, ("bDualMacSmartConcurrent=%d\n",pDM_Odm->bDualMacSmartConcurrent) );

}

void
odm_CmnInfoHook_Debug(
	IN		PDM_ODM_T		pDM_Odm
	)
{
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_COMMON, ODM_DBG_LOUD, ("odm_CmnInfoHook_Debug==>\n"));	
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_COMMON, ODM_DBG_LOUD, ("pNumTxBytesUnicast=%llu\n",*(pDM_Odm->pNumTxBytesUnicast)) );
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_COMMON, ODM_DBG_LOUD, ("pNumRxBytesUnicast=%llu\n",*(pDM_Odm->pNumRxBytesUnicast)) );
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_COMMON, ODM_DBG_LOUD, ("pWirelessMode=0x%x\n",*(pDM_Odm->pWirelessMode)) );	
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_COMMON, ODM_DBG_LOUD, ("pSecChOffset=%d\n",*(pDM_Odm->pSecChOffset)) );
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_COMMON, ODM_DBG_LOUD, ("pSecurity=%d\n",*(pDM_Odm->pSecurity)) );
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_COMMON, ODM_DBG_LOUD, ("pBandWidth=%d\n",*(pDM_Odm->pBandWidth)) );
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_COMMON, ODM_DBG_LOUD, ("pChannel=%d\n",*(pDM_Odm->pChannel)) );

	ODM_RT_TRACE(pDM_Odm,ODM_COMP_COMMON, ODM_DBG_LOUD, ("pbScanInProcess=%d\n",*(pDM_Odm->pbScanInProcess)) );
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_COMMON, ODM_DBG_LOUD, ("pbPowerSaving=%d\n",*(pDM_Odm->pbPowerSaving)) );
}

void
odm_CmnInfoUpdate_Debug(
	IN		PDM_ODM_T		pDM_Odm
	)
{
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_COMMON, ODM_DBG_LOUD, ("odm_CmnInfoUpdate_Debug==>\n"));
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_COMMON, ODM_DBG_LOUD, ("bWIFI_Direct=%d\n",pDM_Odm->bWIFI_Direct) );
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_COMMON, ODM_DBG_LOUD, ("bWIFI_Display=%d\n",pDM_Odm->bWIFI_Display) );
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_COMMON, ODM_DBG_LOUD, ("bLinked=%d\n",pDM_Odm->bLinked) );
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_COMMON, ODM_DBG_LOUD, ("RSSI_Min=%d\n",pDM_Odm->RSSI_Min) );
}

void
odm_BasicDbgMessage
(
	IN		PDM_ODM_T		pDM_Odm
	)
{
	Pfalse_ALARM_STATISTICS FalseAlmCnt = &(pDM_Odm->FalseAlmCnt);
	pDIG_T	pDM_DigTable = &pDM_Odm->DM_DigTable;
	
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_COMMON, ODM_DBG_LOUD, ("odm_BasicDbgMsg==>\n"));
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_COMMON, ODM_DBG_LOUD, ("bLinked = %d, RSSI_Min = %d, CurrentIGI = 0x%x \n",
		pDM_Odm->bLinked, pDM_Odm->RSSI_Min, pDM_DigTable->CurIGValue) );
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_COMMON, ODM_DBG_LOUD, ("Cnt_Cck_fail = %d, Cnt_Ofdm_fail = %d, Total False Alarm = %d\n",	
		FalseAlmCnt->Cnt_Cck_fail, FalseAlmCnt->Cnt_Ofdm_fail, FalseAlmCnt->Cnt_all));
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_COMMON, ODM_DBG_LOUD, ("RxRate = 0x%x, RSSI_A = %d, RSSI_B = %d\n", 
		pDM_Odm->RxRate, pDM_Odm->RSSI_A, pDM_Odm->RSSI_B));
	//ODM_RT_TRACE(pDM_Odm,ODM_COMP_COMMON, ODM_DBG_LOUD, ("RSSI_C = %d, RSSI_D = %d\n", pDM_Odm->RSSI_C, pDM_Odm->RSSI_D));

}

/*
void
odm_FindMinimumRSSI(
	IN		PDM_ODM_T		pDM_Odm
	)
{
	u4Byte	i;
	u1Byte	RSSI_Min = 0xFF;

	for(i=0; i<ODM_ASSOCIATE_ENTRY_NUM; i++)
	{
//		if(pDM_Odm->pODM_StaInfo[i] != NULL)
		if(IS_STA_VALID(pDM_Odm->pODM_StaInfo[i]) )
		{
			if(pDM_Odm->pODM_StaInfo[i]->RSSI_Ave < RSSI_Min)
			{
				RSSI_Min = pDM_Odm->pODM_StaInfo[i]->RSSI_Ave;
			}
		}
	}

	pDM_Odm->RSSI_Min = RSSI_Min;

}

void
odm_IsLinked(
	IN		PDM_ODM_T		pDM_Odm
	)
{
	u4Byte i;
	bool Linked = false;
	
	for(i=0; i<ODM_ASSOCIATE_ENTRY_NUM; i++)
	{
			if(IS_STA_VALID(pDM_Odm->pODM_StaInfo[i]) )
			{			
				Linked = true;
				break;
			}
		
	}

	pDM_Odm->bLinked = Linked;
}
*/


//3============================================================
//3 DIG
//3============================================================
/*-----------------------------------------------------------------------------
 * Function:	odm_DIGInit()
 *
 * Overview:	Set DIG scheme init value.
 *
 * Input:		NONE
 *
 * Output:		NONE
 *
 * Return:		NONE
 *
 * Revised History:
 *	When		Who		Remark
 *
 *---------------------------------------------------------------------------*/

//Remove DIG by yuchen

//Remove DIG and FA check by Yu Chen


//3============================================================
//3 BB Power Save
//3============================================================

//Remove BB power saving by Yuchen

//3============================================================
//3 RATR MASK
//3============================================================
//3============================================================
//3 Rate Adaptive
//3============================================================

void
odm_RateAdaptiveMaskInit(
	IN 	PDM_ODM_T	pDM_Odm
	)
{
	PODM_RATE_ADAPTIVE	pOdmRA = &pDM_Odm->RateAdaptive;

	pOdmRA->Type = DM_Type_ByDriver;
	if (pOdmRA->Type == DM_Type_ByDriver)
		pDM_Odm->bUseRAMask = true;
	else
		pDM_Odm->bUseRAMask = false;	

	pOdmRA->RATRState = DM_RATR_STA_INIT;
	pOdmRA->LdpcThres = 35;
	pOdmRA->bUseLdpc = false;
	pOdmRA->HighRSSIThresh = 50;
	pOdmRA->LowRSSIThresh = 20;
}

u4Byte ODM_Get_Rate_Bitmap(
	IN	PDM_ODM_T	pDM_Odm,	
	IN	u4Byte		macid,
	IN	u4Byte 		ra_mask,	
	IN	u1Byte 		rssi_level)
{
	PSTA_INFO_T   	pEntry;
	u4Byte 	rate_bitmap = 0;
	u1Byte 	WirelessMode;
	//u1Byte 	WirelessMode =*(pDM_Odm->pWirelessMode);
	
	
	pEntry = pDM_Odm->pODM_StaInfo[macid];
	if(!IS_STA_VALID(pEntry))
		return ra_mask;

	WirelessMode = pEntry->wireless_mode;
	
	switch(WirelessMode)
	{
		case ODM_WM_B:
			if(ra_mask & 0x0000000c)		//11M or 5.5M enable				
				rate_bitmap = 0x0000000d;
			else
				rate_bitmap = 0x0000000f;
			break;
			
		case (ODM_WM_G):
		case (ODM_WM_A):
			if(rssi_level == DM_RATR_STA_HIGH)
				rate_bitmap = 0x00000f00;
			else
				rate_bitmap = 0x00000ff0;
			break;
			
		case (ODM_WM_B|ODM_WM_G):
			if(rssi_level == DM_RATR_STA_HIGH)
				rate_bitmap = 0x00000f00;
			else if(rssi_level == DM_RATR_STA_MIDDLE)
				rate_bitmap = 0x00000ff0;
			else
				rate_bitmap = 0x00000ff5;
			break;		

		case (ODM_WM_B|ODM_WM_G|ODM_WM_N24G)	:
		case (ODM_WM_B|ODM_WM_N24G)	:
		case (ODM_WM_G|ODM_WM_N24G)	:
		case (ODM_WM_A|ODM_WM_N5G)	:
			{					
				if (	pDM_Odm->RFType == ODM_1T2R ||pDM_Odm->RFType == ODM_1T1R)
				{
					if(rssi_level == DM_RATR_STA_HIGH)
					{
						rate_bitmap = 0x000f0000;
					}
					else if(rssi_level == DM_RATR_STA_MIDDLE)
					{
						rate_bitmap = 0x000ff000;
					}
					else{
						if (*(pDM_Odm->pBandWidth) == ODM_BW40M)
							rate_bitmap = 0x000ff015;
						else
							rate_bitmap = 0x000ff005;
					}				
				}
				else
				{
					if(rssi_level == DM_RATR_STA_HIGH)
					{		
						rate_bitmap = 0x0f8f0000;
					}
					else if(rssi_level == DM_RATR_STA_MIDDLE)
					{
						rate_bitmap = 0x0f8ff000;
					}
					else
					{
						if (*(pDM_Odm->pBandWidth) == ODM_BW40M)
							rate_bitmap = 0x0f8ff015;
						else
							rate_bitmap = 0x0f8ff005;
					}					
				}
			}
			break;

		case (ODM_WM_AC|ODM_WM_G):
			if(rssi_level == 1)
				rate_bitmap = 0xfc3f0000;
			else if(rssi_level == 2)
				rate_bitmap = 0xfffff000;
			else
				rate_bitmap = 0xffffffff;
			break;

		case (ODM_WM_AC|ODM_WM_A):

			if (pDM_Odm->RFType == RF_1T1R)
			{
				if(rssi_level == 1)				// add by Gary for ac-series
					rate_bitmap = 0x003f8000;
				else if (rssi_level == 2)
					rate_bitmap = 0x003ff000;
				else
					rate_bitmap = 0x003ff010;
			}
			else
			{
				if(rssi_level == 1)				// add by Gary for ac-series
					rate_bitmap = 0xfe3f8000;       // VHT 2SS MCS3~9
				else if (rssi_level == 2)
					rate_bitmap = 0xfffff000;       // VHT 2SS MCS0~9
				else
					rate_bitmap = 0xfffff010;       // All
			}
			break;
			
		default:
			if(pDM_Odm->RFType == RF_1T2R)
				rate_bitmap = 0x000fffff;
			else
				rate_bitmap = 0x0fffffff;
			break;	

	}

	//printk("%s ==> rssi_level:0x%02x, WirelessMode:0x%02x, rate_bitmap:0x%08x \n",__FUNCTION__,rssi_level,WirelessMode,rate_bitmap);
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_RA_MASK, ODM_DBG_LOUD, (" ==> rssi_level:0x%02x, WirelessMode:0x%02x, rate_bitmap:0x%08x \n",rssi_level,WirelessMode,rate_bitmap));

	return (ra_mask&rate_bitmap);
	
}	

void
odm_RefreshBasicRateMask(
	IN		PDM_ODM_T		pDM_Odm	
	)
{
}

/*-----------------------------------------------------------------------------
 * Function:	odm_RefreshRateAdaptiveMask()
 *
 * Overview:	Update rate table mask according to rssi
 *
 * Input:		NONE
 *
 * Output:		NONE
 *
 * Return:		NONE
 *
 * Revised History:
 *	When		Who		Remark
 *	05/27/2009	hpfan	Create Version 0.  
 *
 *---------------------------------------------------------------------------*/
void
odm_RefreshRateAdaptiveMask(
	IN		PDM_ODM_T		pDM_Odm
	)
{

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_RA_MASK, ODM_DBG_TRACE, ("odm_RefreshRateAdaptiveMask()---------->\n"));	
	if (!(pDM_Odm->SupportAbility & ODM_BB_RA_MASK))
	{
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_RA_MASK, ODM_DBG_TRACE, ("odm_RefreshRateAdaptiveMask(): Return cos not supported\n"));
		return;	
	}
	odm_RefreshRateAdaptiveMaskCE(pDM_Odm);
}

void
odm_RefreshRateAdaptiveMaskMP(
	IN		PDM_ODM_T		pDM_Odm	
	)
{
}


void
odm_RefreshRateAdaptiveMaskCE(
	IN		PDM_ODM_T		pDM_Odm	
	)
{
	u1Byte	i;
	PADAPTER	pAdapter	 =  pDM_Odm->Adapter;
	PODM_RATE_ADAPTIVE		pRA = &pDM_Odm->RateAdaptive;

	if(pAdapter->bDriverStopped)
	{
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_RA_MASK, ODM_DBG_TRACE, ("<---- odm_RefreshRateAdaptiveMask(): driver is going to unload\n"));
		return;
	}

	if(!pDM_Odm->bUseRAMask)
	{
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_RA_MASK, ODM_DBG_LOUD, ("<---- odm_RefreshRateAdaptiveMask(): driver does not control rate adaptive mask\n"));
		return;
	}

	//printk("==> %s \n",__FUNCTION__);

	for(i=0; i<ODM_ASSOCIATE_ENTRY_NUM; i++){
		PSTA_INFO_T pstat = pDM_Odm->pODM_StaInfo[i];
		if(IS_STA_VALID(pstat) ) {
			if(IS_MCAST( pstat->hwaddr))  //if(psta->mac_id ==1)
				 continue;
			if(IS_MCAST( pstat->hwaddr))
				continue;

			if( true == ODM_RAStateCheck(pDM_Odm, pstat->rssi_stat.UndecoratedSmoothedPWDB, false , &pstat->rssi_level) )
			{
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_RA_MASK, ODM_DBG_LOUD, ("RSSI:%d, RSSI_LEVEL:%d\n", pstat->rssi_stat.UndecoratedSmoothedPWDB, pstat->rssi_level));
				//printk("RSSI:%d, RSSI_LEVEL:%d\n", pstat->rssi_stat.UndecoratedSmoothedPWDB, pstat->rssi_level);
				rtw_hal_update_ra_mask(pstat, pstat->rssi_level);
			}
		
		}
	}			
}

void
odm_RefreshRateAdaptiveMaskAPADSL(
	IN		PDM_ODM_T		pDM_Odm
	)
{
}

// Return Value: bool
// - true: RATRState is changed.
bool 
ODM_RAStateCheck(
	IN		PDM_ODM_T		pDM_Odm,
	IN		s4Byte			RSSI,
	IN		bool			bForceUpdate,
	OUT		pu1Byte			pRATRState
	)
{
	PODM_RATE_ADAPTIVE pRA = &pDM_Odm->RateAdaptive;
	const u1Byte GoUpGap = 5;
	u1Byte HighRSSIThreshForRA = pRA->HighRSSIThresh;
	u1Byte LowRSSIThreshForRA = pRA->LowRSSIThresh;
	u1Byte RATRState;

	// Threshold Adjustment: 
	// when RSSI state trends to go up one or two levels, make sure RSSI is high enough.
	// Here GoUpGap is added to solve the boundary's level alternation issue.
	switch (*pRATRState)
	{
		case DM_RATR_STA_INIT:
		case DM_RATR_STA_HIGH:
			break;

		case DM_RATR_STA_MIDDLE:
			HighRSSIThreshForRA += GoUpGap;
			break;

		case DM_RATR_STA_LOW:
			HighRSSIThreshForRA += GoUpGap;
			LowRSSIThreshForRA += GoUpGap;
			break;

		default: 
			ODM_RT_ASSERT(pDM_Odm, false, ("wrong rssi level setting %d !", *pRATRState) );
			break;
	}

	// Decide RATRState by RSSI.
	if(RSSI > HighRSSIThreshForRA)
		RATRState = DM_RATR_STA_HIGH;
	else if(RSSI > LowRSSIThreshForRA)
		RATRState = DM_RATR_STA_MIDDLE;
	else
		RATRState = DM_RATR_STA_LOW;
	//printk("==>%s,RATRState:0x%02x ,RSSI:%d \n",__FUNCTION__,RATRState,RSSI);

	if( *pRATRState!=RATRState || bForceUpdate)
	{
		ODM_RT_TRACE( pDM_Odm, ODM_COMP_RA_MASK, ODM_DBG_LOUD, ("RSSI Level %d -> %d\n", *pRATRState, RATRState) );
		*pRATRState = RATRState;
		return true;
	}

	return false;
}


//============================================================

//3============================================================
//3 Dynamic Tx Power
//3============================================================

//Remove BY YuChen

//3============================================================
//3 RSSI Monitor
//3============================================================

void
odm_RSSIMonitorInit(
	IN	PDM_ODM_T	pDM_Odm
	)
{
	pRA_T		pRA_Table = &pDM_Odm->DM_RA_Table;

   	pRA_Table->firstconnect = false;

}

void
odm_RSSIMonitorCheck(
	IN		PDM_ODM_T		pDM_Odm
	)
{
	if (!(pDM_Odm->SupportAbility & ODM_BB_RSSI_MONITOR))
		return;

	odm_RSSIMonitorCheckCE(pDM_Odm);

}	// odm_RSSIMonitorCheck


void
odm_RSSIMonitorCheckMP(
	IN	PDM_ODM_T	pDM_Odm
	)
{
}

static void
FindMinimumRSSI(
IN	PADAPTER	pAdapter
	)
{	
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(pAdapter);
	struct dm_priv	*pdmpriv = &pHalData->dmpriv;	
	PDM_ODM_T		pDM_Odm = &(pHalData->odmpriv);

	//1 1.Determine the minimum RSSI 

	if((pDM_Odm->bLinked != true) &&
		(pdmpriv->EntryMinUndecoratedSmoothedPWDB == 0))
	{
		pdmpriv->MinUndecoratedPWDBForDM = 0;
		//ODM_RT_TRACE(pDM_Odm,COMP_BB_POWERSAVING, DBG_LOUD, ("Not connected to any \n"));
	}
	else
	{
		pdmpriv->MinUndecoratedPWDBForDM = pdmpriv->EntryMinUndecoratedSmoothedPWDB;
	}

	//DBG_8192C("%s=>MinUndecoratedPWDBForDM(%d)\n",__FUNCTION__,pdmpriv->MinUndecoratedPWDBForDM);
	//ODM_RT_TRACE(pDM_Odm,COMP_DIG, DBG_LOUD, ("MinUndecoratedPWDBForDM =%d\n",pHalData->MinUndecoratedPWDBForDM));
}

void
odm_RSSIMonitorCheckCE(
	IN		PDM_ODM_T		pDM_Odm
	)
{
	PADAPTER	Adapter = pDM_Odm->Adapter;
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);
	struct dm_priv	*pdmpriv = &pHalData->dmpriv;
	struct dvobj_priv	*pdvobjpriv = adapter_to_dvobj(Adapter);
	int	i;
	int	tmpEntryMaxPWDB=0, tmpEntryMinPWDB=0xff;
	u8 	sta_cnt=0;
	u32	UL_DL_STATE = 0, STBC_TX = 0, TxBF_EN = 0;
	u32	PWDB_rssi[NUM_STA]={0};//[0~15]:MACID, [16~31]:PWDB_rssi
	bool			FirstConnect = false;
	pRA_T			pRA_Table = &pDM_Odm->DM_RA_Table;

	if(pDM_Odm->bLinked != true)
		return;

       FirstConnect = (pDM_Odm->bLinked) && (pRA_Table->firstconnect == false);    
	pRA_Table->firstconnect = pDM_Odm->bLinked;

	//if(check_fwstate(&Adapter->mlmepriv, WIFI_AP_STATE|WIFI_ADHOC_STATE|WIFI_ADHOC_MASTER_STATE) == true)
	{
		struct sta_info *psta;
		
		for(i=0; i<ODM_ASSOCIATE_ENTRY_NUM; i++) {
			if (IS_STA_VALID(psta = pDM_Odm->pODM_StaInfo[i]))
			{
                        		if(IS_MCAST( psta->hwaddr))  //if(psta->mac_id ==1)
						 continue;
								
					if(psta->rssi_stat.UndecoratedSmoothedPWDB == (-1))
						 continue;
								
					if(psta->rssi_stat.UndecoratedSmoothedPWDB < tmpEntryMinPWDB)
						tmpEntryMinPWDB = psta->rssi_stat.UndecoratedSmoothedPWDB;

					if(psta->rssi_stat.UndecoratedSmoothedPWDB > tmpEntryMaxPWDB)
						tmpEntryMaxPWDB = psta->rssi_stat.UndecoratedSmoothedPWDB;

					if(psta->rssi_stat.UndecoratedSmoothedPWDB != (-1)) {
						PWDB_rssi[sta_cnt++] = (psta->mac_id | (psta->rssi_stat.UndecoratedSmoothedPWDB<<16) );
					}
			}
		}

		//printk("%s==> sta_cnt(%d)\n",__FUNCTION__,sta_cnt);

		for(i=0; i< sta_cnt; i++)
		{
			if(PWDB_rssi[i] != (0)){
				if(pHalData->fw_ractrl == true)// Report every sta's RSSI to FW
				{
					rtl8723b_set_rssi_cmd(Adapter, (u8 *)(&PWDB_rssi[i]));
				}
			}
		}		
	}



	if(tmpEntryMaxPWDB != 0)	// If associated entry is found
	{
		pdmpriv->EntryMaxUndecoratedSmoothedPWDB = tmpEntryMaxPWDB;		
	}
	else
	{
		pdmpriv->EntryMaxUndecoratedSmoothedPWDB = 0;
	}

	if(tmpEntryMinPWDB != 0xff) // If associated entry is found
	{
		pdmpriv->EntryMinUndecoratedSmoothedPWDB = tmpEntryMinPWDB;		
	}
	else
	{
		pdmpriv->EntryMinUndecoratedSmoothedPWDB = 0;
	}

	FindMinimumRSSI(Adapter);//get pdmpriv->MinUndecoratedPWDBForDM

	pDM_Odm->RSSI_Min = pdmpriv->MinUndecoratedPWDBForDM;
	//ODM_CmnInfoUpdate(&pHalData->odmpriv ,ODM_CMNINFO_RSSI_MIN, pdmpriv->MinUndecoratedPWDBForDM);
}

void
odm_RSSIMonitorCheckAP(
	IN		PDM_ODM_T		pDM_Odm
	)
{
}



void
ODM_InitAllTimers(
	IN PDM_ODM_T	pDM_Odm 
	)
{
}

void
ODM_CancelAllTimers(
	IN PDM_ODM_T	pDM_Odm 
	)
{
}


void
ODM_ReleaseAllTimers(
	IN PDM_ODM_T	pDM_Odm 
	)
{
}


//3============================================================
//3 Tx Power Tracking
//3============================================================

void
odm_TXPowerTrackingInit(
	IN	PDM_ODM_T	pDM_Odm 
	)
{
	odm_TXPowerTrackingThermalMeterInit(pDM_Odm);
}	

static u1Byte
getSwingIndex(
	IN	PDM_ODM_T	pDM_Odm 
	)
{
	PADAPTER		Adapter = pDM_Odm->Adapter;
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);
	u1Byte 			i = 0;
	u4Byte 			bbSwing;
	u4Byte 			swingTableSize;
	pu4Byte 			pSwingTable;

	bbSwing = PHY_QueryBBReg(Adapter, rOFDM0_XATxIQImbalance, 0xFFC00000);

	pSwingTable = OFDMSwingTable_New;
	swingTableSize = OFDM_TABLE_SIZE;

	for (i = 0; i < swingTableSize; ++i) {
		u4Byte tableValue = pSwingTable[i];
		
		if (tableValue >= 0x100000 )
			tableValue >>= 22;
		if (bbSwing == tableValue)
			break;
	}
	return i;
}

void
odm_TXPowerTrackingThermalMeterInit(
	IN	PDM_ODM_T	pDM_Odm 
	)
{
	u1Byte defaultSwingIndex = getSwingIndex(pDM_Odm);
	u1Byte 			p = 0;
	PADAPTER			Adapter = pDM_Odm->Adapter;
	HAL_DATA_TYPE		*pHalData = GET_HAL_DATA(Adapter);


	struct dm_priv	*pdmpriv = &pHalData->dmpriv;

	pdmpriv->bTXPowerTracking = true;
	pdmpriv->TXPowercount = 0;
	pdmpriv->bTXPowerTrackingInit = false;
	//#if	(MP_DRIVER != 1)		//for mp driver, turn off txpwrtracking as default

	if (*(pDM_Odm->mp_mode) != 1)
		pdmpriv->TxPowerTrackControl = true;
	else
		pdmpriv->TxPowerTrackControl = false;


	//MSG_8192C("pdmpriv->TxPowerTrackControl = %d\n", pdmpriv->TxPowerTrackControl);

	//pDM_Odm->RFCalibrateInfo.TxPowerTrackControl = true;
	pDM_Odm->RFCalibrateInfo.ThermalValue = pHalData->EEPROMThermalMeter;
	pDM_Odm->RFCalibrateInfo.ThermalValue_IQK = pHalData->EEPROMThermalMeter;
	pDM_Odm->RFCalibrateInfo.ThermalValue_LCK = pHalData->EEPROMThermalMeter;	

	// The index of "0 dB" in SwingTable.
	pDM_Odm->DefaultOfdmIndex = (defaultSwingIndex >= OFDM_TABLE_SIZE) ? 30 : defaultSwingIndex;
	pDM_Odm->DefaultCckIndex = 20;	

	pDM_Odm->BbSwingIdxCckBase = pDM_Odm->DefaultCckIndex;
	pDM_Odm->RFCalibrateInfo.CCK_index = pDM_Odm->DefaultCckIndex;
	
	for (p = ODM_RF_PATH_A; p < MAX_RF_PATH; ++p)
	{
		pDM_Odm->BbSwingIdxOfdmBase[p] = pDM_Odm->DefaultOfdmIndex;		
	   	pDM_Odm->RFCalibrateInfo.OFDM_index[p] = pDM_Odm->DefaultOfdmIndex;		
		pDM_Odm->RFCalibrateInfo.DeltaPowerIndex[p] = 0;
		pDM_Odm->RFCalibrateInfo.DeltaPowerIndexLast[p] = 0;
		pDM_Odm->RFCalibrateInfo.PowerIndexOffset[p] = 0;
	}

}


void
ODM_TXPowerTrackingCheck(
	IN		PDM_ODM_T		pDM_Odm
	)
{
	odm_TXPowerTrackingCheckCE(pDM_Odm);
}

void
odm_TXPowerTrackingCheckCE(
	IN		PDM_ODM_T		pDM_Odm 
	)
{
	PADAPTER	Adapter = pDM_Odm->Adapter;

	if(!(pDM_Odm->SupportAbility & ODM_RF_TX_PWR_TRACK))
	{
		return;
	}

	if(!pDM_Odm->RFCalibrateInfo.TM_Trigger)		//at least delay 1 sec
	{
		//pHalData->TxPowerCheckCnt++;	//cosa add for debug
		ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, RF_T_METER_NEW, (BIT17 | BIT16), 0x03);

		//DBG_871X("Trigger Thermal Meter!!\n");
		
		pDM_Odm->RFCalibrateInfo.TM_Trigger = 1;
		return;
	}
	else
	{
		//DBG_871X("Schedule TxPowerTracking direct call!!\n");
		ODM_TXPowerTrackingCallback_ThermalMeter(Adapter);
		pDM_Odm->RFCalibrateInfo.TM_Trigger = 0;
	}
}

void
odm_TXPowerTrackingCheckMP(
	IN		PDM_ODM_T		pDM_Odm 
	)
{
}


void
odm_TXPowerTrackingCheckAP(
	IN		PDM_ODM_T		pDM_Odm
	)
{
}

//3============================================================
//3 SW Antenna Diversity
//3============================================================
void
odm_AntennaDiversityInit(
	IN 		PDM_ODM_T		pDM_Odm 
)
{
	if(*(pDM_Odm->mp_mode) == true)
		return;
}

void
odm_AntennaDiversity(
	IN 		PDM_ODM_T		pDM_Odm 
)
{
	if(*(pDM_Odm->mp_mode) == true)
		return;
}


void
odm_SwAntDetectInit(
	IN		PDM_ODM_T		pDM_Odm
	)
{
	pSWAT_T		pDM_SWAT_Table = &pDM_Odm->DM_SWAT_Table;
	pDM_SWAT_Table->SWAS_NoLink_BK_Reg92c = ODM_Read4Byte(pDM_Odm, rDPDT_control);
	pDM_SWAT_Table->PreAntenna = MAIN_ANT;
	pDM_SWAT_Table->CurAntenna = MAIN_ANT;
	pDM_SWAT_Table->SWAS_NoLink_State = 0;
}

//============================================================
//EDCA Turbo
//============================================================

// need to ODM CE Platform
//move to here for ANT detection mechanism using

u4Byte
GetPSDData(
	IN PDM_ODM_T	pDM_Odm,
	unsigned int 	point,
	u1Byte initial_gain_psd)
{
	//unsigned int	val, rfval;
	//int	psd_report;
	u4Byte	psd_report;
	
	//HAL_DATA_TYPE		*pHalData = GET_HAL_DATA(Adapter);
	//Debug Message
	//val = PHY_QueryBBReg(Adapter,0x908, bMaskDWord);
	//DbgPrint("Reg908 = 0x%x\n",val);
	//val = PHY_QueryBBReg(Adapter,0xDF4, bMaskDWord);
	//rfval = PHY_QueryRFReg(Adapter, ODM_RF_PATH_A, 0x00, bRFRegOffsetMask);
	//DbgPrint("RegDF4 = 0x%x, RFReg00 = 0x%x\n",val, rfval);
	//DbgPrint("PHYTXON = %x, OFDMCCA_PP = %x, CCKCCA_PP = %x, RFReg00 = %x\n",
		//(val&BIT25)>>25, (val&BIT14)>>14, (val&BIT15)>>15, rfval);

	//Set DCO frequency index, offset=(40MHz/SamplePts)*point
	ODM_SetBBReg(pDM_Odm, 0x808, 0x3FF, point);

	//Start PSD calculation, Reg808[22]=0->1
	ODM_SetBBReg(pDM_Odm, 0x808, BIT22, 1);
	//Need to wait for HW PSD report
	udelay(1000);
	ODM_SetBBReg(pDM_Odm, 0x808, BIT22, 0);
	//Read PSD report, Reg8B4[15:0]
	psd_report = ODM_GetBBReg(pDM_Odm,0x8B4, bMaskDWord) & 0x0000FFFF;
	
	psd_report = (u4Byte) (ConvertTo_dB(psd_report))+(u4Byte)(initial_gain_psd-0x1c);

	return psd_report;
	
}

u4Byte 
ConvertTo_dB(
	u4Byte 	Value)
{
	u1Byte i;
	u1Byte j;
	u4Byte dB;

	Value = Value & 0xFFFF;
	
	for (i=0;i<8;i++)
	{
		if (Value <= dB_Invert_Table[i][11])
		{
			break;
		}
	}

	if (i >= 8)
	{
		return (96);	// maximum 96 dB
	}

	for (j=0;j<12;j++)
	{
		if (Value <= dB_Invert_Table[i][j])
		{
			break;
		}
	}

	dB = i*12 + j + 1;

	return (dB);
}

//
// Description:
//	Set Single/Dual Antenna default setting for products that do not do detection in advance.
//
// Added by Joseph, 2012.03.22
//
void
ODM_SingleDualAntennaDefaultSetting(
	IN		PDM_ODM_T		pDM_Odm
	)
{
	pSWAT_T		pDM_SWAT_Table = &pDM_Odm->DM_SWAT_Table;
	PADAPTER	pAdapter	 =  pDM_Odm->Adapter;
	u1Byte btAntNum = 2;

	btAntNum = hal_btcoex_GetPgAntNum(pAdapter);

	// Set default antenna A and B status
	if(btAntNum == 2)
	{
		pDM_SWAT_Table->ANTA_ON=true;
		pDM_SWAT_Table->ANTB_ON=true;
		//RT_TRACE(COMP_ANTENNA, DBG_LOUD, ("Dual antenna\n"));
	}
	else if(btAntNum == 1)
	{// Set antenna A as default
		pDM_SWAT_Table->ANTA_ON=true;
		pDM_SWAT_Table->ANTB_ON=false;
		//RT_TRACE(COMP_ANTENNA, DBG_LOUD, ("Single antenna\n"));
	}
	else
	{
		//RT_ASSERT(false, ("Incorrect antenna number!!\n"));
	}
}



//2 8723A ANT DETECT
//
// Description:
//	Implement IQK single tone for RF DPK loopback and BB PSD scanning. 
//	This function is cooperated with BB team Neil. 
//
// Added by Roger, 2011.12.15
//
bool
ODM_SingleDualAntennaDetection(
	IN		PDM_ODM_T		pDM_Odm,
	IN		u1Byte			mode
	)
{
	PADAPTER	pAdapter	 =  pDM_Odm->Adapter;
	pSWAT_T		pDM_SWAT_Table = &pDM_Odm->DM_SWAT_Table;
	u4Byte		CurrentChannel,RfLoopReg;
	u1Byte		n;
	u4Byte		Reg88c, Regc08, Reg874, Regc50, Reg948=0, Regb2c=0, Reg92c=0, AFE_rRx_Wait_CCA=0;
	u1Byte		initial_gain = 0x5a;
	u4Byte		PSD_report_tmp;
	u4Byte		AntA_report = 0x0, AntB_report = 0x0,AntO_report=0x0;
	bool		bResult = true;
	u4Byte		AFE_Backup[16];
	u4Byte		AFE_REG_8723A[16] = {
					rRx_Wait_CCA, 	rTx_CCK_RFON, 
					rTx_CCK_BBON, 	rTx_OFDM_RFON,
					rTx_OFDM_BBON, 	rTx_To_Rx,
					rTx_To_Tx, 		rRx_CCK, 
					rRx_OFDM, 		rRx_Wait_RIFS, 
					rRx_TO_Rx,		rStandby,
					rSleep,			rPMPD_ANAEN, 	
					rFPGA0_XCD_SwitchControl, rBlue_Tooth};

	ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("ODM_SingleDualAntennaDetection()============> \n"));	

	
	// Retrieve antenna detection registry info, added by Roger, 2012.11.27.
	if(!IS_ANT_DETECT_SUPPORT_SINGLE_TONE(pAdapter))
		return bResult;

	//1 Backup Current RF/BB Settings	
	
	CurrentChannel = ODM_GetRFReg(pDM_Odm, ODM_RF_PATH_A, ODM_CHANNEL, bRFRegOffsetMask);
	RfLoopReg = ODM_GetRFReg(pDM_Odm, ODM_RF_PATH_A, 0x00, bRFRegOffsetMask);

	Reg92c = ODM_GetBBReg(pDM_Odm, 0x92c, bMaskDWord);
	Reg948 = ODM_GetBBReg(pDM_Odm, rS0S1_PathSwitch, bMaskDWord);
	Regb2c = ODM_GetBBReg(pDM_Odm, AGC_table_select, bMaskDWord);
	ODM_SetBBReg(pDM_Odm, rDPDT_control, 0x3, 0x1);
	ODM_SetBBReg(pDM_Odm, rfe_ctrl_anta_src, 0xff, 0x77);
	ODM_SetBBReg(pDM_Odm, rS0S1_PathSwitch, 0x3ff, 0x000);
	ODM_SetBBReg(pDM_Odm, AGC_table_select, BIT31, 0x0);

	udelay(10);
	
	//Store A Path Register 88c, c08, 874, c50
	Reg88c = ODM_GetBBReg(pDM_Odm, rFPGA0_AnalogParameter4, bMaskDWord);
	Regc08 = ODM_GetBBReg(pDM_Odm, rOFDM0_TRMuxPar, bMaskDWord);
	Reg874 = ODM_GetBBReg(pDM_Odm, rFPGA0_XCD_RFInterfaceSW, bMaskDWord);
	Regc50 = ODM_GetBBReg(pDM_Odm, rOFDM0_XAAGCCore1, bMaskDWord);	
	
	// Store AFE Registers
	AFE_rRx_Wait_CCA = ODM_GetBBReg(pDM_Odm, rRx_Wait_CCA,bMaskDWord);
	
	//Set PSD 128 pts
	ODM_SetBBReg(pDM_Odm, rFPGA0_PSDFunction, BIT14|BIT15, 0x0);  //128 pts
	
	// To SET CH1 to do
	ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, ODM_CHANNEL, bRFRegOffsetMask, 0x7401);     //Channel 1
	
	// AFE all on step
	ODM_SetBBReg(pDM_Odm, rRx_Wait_CCA, bMaskDWord, 0x01c00016);

	// 3 wire Disable
	ODM_SetBBReg(pDM_Odm, rFPGA0_AnalogParameter4, bMaskDWord, 0xCCF000C0);
	
	//BB IQK Setting
	ODM_SetBBReg(pDM_Odm, rOFDM0_TRMuxPar, bMaskDWord, 0x000800E4);
	ODM_SetBBReg(pDM_Odm, rFPGA0_XCD_RFInterfaceSW, bMaskDWord, 0x22208000);

	//IQK setting tone@ 4.34Mhz
	ODM_SetBBReg(pDM_Odm, rTx_IQK_Tone_A, bMaskDWord, 0x10008C1C);
	ODM_SetBBReg(pDM_Odm, rTx_IQK, bMaskDWord, 0x01007c00);	

	//Page B init
	ODM_SetBBReg(pDM_Odm, rConfig_AntA, bMaskDWord, 0x00080000);
	ODM_SetBBReg(pDM_Odm, rConfig_AntA, bMaskDWord, 0x0f600000);
	ODM_SetBBReg(pDM_Odm, rRx_IQK, bMaskDWord, 0x01004800);
	ODM_SetBBReg(pDM_Odm, rRx_IQK_Tone_A, bMaskDWord, 0x10008c1f);
	ODM_SetBBReg(pDM_Odm, rTx_IQK_PI_A, bMaskDWord, 0x82150008);
	ODM_SetBBReg(pDM_Odm, rRx_IQK_PI_A, bMaskDWord, 0x28150008);
	ODM_SetBBReg(pDM_Odm, rIQK_AGC_Rsp, bMaskDWord, 0x001028d0);	

	//IQK Single tone start
	ODM_SetBBReg(pDM_Odm, rFPGA0_IQK, bMaskH3Bytes, 0x808000);
	ODM_SetBBReg(pDM_Odm, rIQK_AGC_Pts, bMaskDWord, 0xf9000000);
	ODM_SetBBReg(pDM_Odm, rIQK_AGC_Pts, bMaskDWord, 0xf8000000);
	
	udelay(10000);

	// PSD report of antenna A
	PSD_report_tmp=0x0;
	for (n=0;n<2;n++)
 	{
 		PSD_report_tmp =  GetPSDData(pDM_Odm, 14, initial_gain);	
		if(PSD_report_tmp >AntA_report)
			AntA_report=PSD_report_tmp;
	}

	 // change to Antenna B
	ODM_SetBBReg(pDM_Odm, rDPDT_control, 0x3, 0x2);

	udelay(10);	

	// PSD report of antenna B
	PSD_report_tmp=0x0;
	for (n=0;n<2;n++)
 	{
 		PSD_report_tmp =  GetPSDData(pDM_Odm, 14, initial_gain);	
		if(PSD_report_tmp > AntB_report)
			AntB_report=PSD_report_tmp;
	}

	// change to open case
	ODM_SetBBReg(pDM_Odm, rDPDT_control, 0x3, 0x0);

	udelay(10);	
	
	// PSD report of open case
	PSD_report_tmp=0x0;
	for (n=0;n<2;n++)
 	{
 		PSD_report_tmp =  GetPSDData(pDM_Odm, 14, initial_gain);	
		if(PSD_report_tmp > AntO_report)
			AntO_report=PSD_report_tmp;
	}

	//Close IQK Single Tone function
	ODM_SetBBReg(pDM_Odm, rFPGA0_IQK, bMaskH3Bytes, 0x000000);	

	// external DPDT
	ODM_SetBBReg(pDM_Odm, rDPDT_control, bMaskDWord, Reg92c);

	//internal S0/S1
	ODM_SetBBReg(pDM_Odm, rS0S1_PathSwitch, bMaskDWord, Reg948);
	ODM_SetBBReg(pDM_Odm, AGC_table_select, bMaskDWord, Regb2c);

	ODM_SetBBReg(pDM_Odm, rFPGA0_AnalogParameter4, bMaskDWord, Reg88c);
	ODM_SetBBReg(pDM_Odm, rOFDM0_TRMuxPar, bMaskDWord, Regc08);
	ODM_SetBBReg(pDM_Odm, rFPGA0_XCD_RFInterfaceSW, bMaskDWord, Reg874);
	ODM_SetBBReg(pDM_Odm, rOFDM0_XAAGCCore1, 0x7F, 0x40);
	ODM_SetBBReg(pDM_Odm, rOFDM0_XAAGCCore1, bMaskDWord, Regc50);
	ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, RF_CHNLBW, bRFRegOffsetMask,CurrentChannel);
	ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, 0x00, bRFRegOffsetMask,RfLoopReg);

	//Reload AFE Registers
	ODM_SetBBReg(pDM_Odm, rRx_Wait_CCA, bMaskDWord, AFE_rRx_Wait_CCA);

	ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("psd_report_A[%d]= %d \n", 2416, AntA_report));	
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("psd_report_B[%d]= %d \n", 2416, AntB_report));	
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("psd_report_O[%d]= %d \n", 2416, AntO_report));
	
	//2 Test Ant B based on Ant A is ON
	if(mode==ANTTESTB)
	{
		if(AntA_report >=100 && AntA_report <= 116)
		{
			if(AntB_report >= (AntA_report+4) && AntB_report > 116)
			{
				pDM_SWAT_Table->ANTB_ON=false;
				ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("ODM_SingleDualAntennaDetection(): Single Antenna A\n"));		
			}	
			else if(AntB_report >=100 && AntB_report <= 116)
			{
				pDM_SWAT_Table->ANTB_ON=true;
				ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("ODM_SingleDualAntennaDetection(): Dual Antenna is A and B\n"));	
			}
			else
			{
				ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("ODM_SingleDualAntennaDetection(): Need to check again\n"));
				pDM_SWAT_Table->ANTB_ON=false; // Set Antenna B off as default 
				bResult = false;
			}
		}
		else
		{
			ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("ODM_SingleDualAntennaDetection(): Need to check again\n"));
			pDM_SWAT_Table->ANTB_ON=false; // Set Antenna B off as default 
			bResult = false;
		}
	}	
	//2 Test Ant A and B based on DPDT Open
	else if(mode==ANTTESTALL)
	{
		if((AntA_report >= 100) && (AntB_report >= 100) && (AntA_report <= 120) && (AntB_report <= 120))
		{
			if((AntA_report - AntB_report < 2) || (AntB_report - AntA_report < 2))
			{
				pDM_SWAT_Table->ANTA_ON=true;
				pDM_SWAT_Table->ANTB_ON=true;
				ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD,("ODM_SingleDualAntennaDetection(): Dual Antenna\n"));
			}
			else if(((AntA_report - AntB_report >= 2) && (AntA_report - AntB_report <= 4)) || 
				((AntB_report - AntA_report >= 2) && (AntB_report - AntA_report <= 4)))
			{
				pDM_SWAT_Table->ANTA_ON=false;
				pDM_SWAT_Table->ANTB_ON=false;
				bResult = false;
				ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("ODM_SingleDualAntennaDetection(): Need to check again\n"));
			}
			else
			{
				pDM_SWAT_Table->ANTA_ON = true;
				pDM_SWAT_Table->ANTB_ON=false;
				ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD,("ODM_SingleDualAntennaDetection(): Single Antenna A\n"));
			}
			
			pDM_Odm->AntDetectedInfo.bAntDetected= true;
			pDM_Odm->AntDetectedInfo.dBForAntA = AntA_report;
			pDM_Odm->AntDetectedInfo.dBForAntB = AntB_report;
			pDM_Odm->AntDetectedInfo.dBForAntO = AntO_report;
			
		}
		else
		{
			ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD,("return false!!\n"));
			bResult = false;
		}
	}

	return bResult;

}

static void
odm_Set_RA_DM_ARFB_by_Noisy(
	IN	PDM_ODM_T	pDM_Odm
)
{
	//DbgPrint("DM_ARFB ====> \n");
	if (pDM_Odm->bNoisyState){
		ODM_Write4Byte(pDM_Odm,0x430,0x00000000);
		ODM_Write4Byte(pDM_Odm,0x434,0x05040200);
		//DbgPrint("DM_ARFB ====> Noisy State\n");
	}
	else{
		ODM_Write4Byte(pDM_Odm,0x430,0x02010000);
		ODM_Write4Byte(pDM_Odm,0x434,0x07050403);
		//DbgPrint("DM_ARFB ====> Clean State\n");
	}
	
}

void
ODM_UpdateNoisyState(
	IN	PDM_ODM_T	pDM_Odm,
	IN 	bool 	bNoisyStateFromC2H
	)
{
	//DbgPrint("Get C2H Command! NoisyState=0x%x\n ", bNoisyStateFromC2H);
	pDM_Odm->bNoisyState = bNoisyStateFromC2H;
	odm_Set_RA_DM_ARFB_by_Noisy(pDM_Odm);
};

u4Byte
Set_RA_DM_Ratrbitmap_by_Noisy(
	IN	PDM_ODM_T	pDM_Odm,
	IN	WIRELESS_MODE	WirelessMode,
	IN	u4Byte			ratr_bitmap,
	IN	u1Byte			rssi_level
)
{
	u4Byte ret_bitmap = ratr_bitmap;
	switch (WirelessMode)
	{
		case WIRELESS_MODE_AC_24G :
		case WIRELESS_MODE_AC_5G :
		case WIRELESS_MODE_AC_ONLY:
			if (pDM_Odm->bNoisyState){ // in Noisy State
				if (rssi_level==1)
					ret_bitmap&=0xfe3f0e08;
				else if (rssi_level==2)
					ret_bitmap&=0xff3f8f8c;
				else if (rssi_level==3)
					ret_bitmap&=0xffffffcc ;
				else
					ret_bitmap&=0xffffffff ;
			}
			else{                                   // in SNR State
				if (rssi_level==1){
					ret_bitmap&=0xfc3e0c08;
				}
				else if (rssi_level==2){
					ret_bitmap&=0xfe3f0e08;
				}
				else if (rssi_level==3){
					ret_bitmap&=0xffbfefcc;
				}
				else{
					ret_bitmap&=0x0fffffff;
				}
			}
			break;
		case WIRELESS_MODE_B:
		case WIRELESS_MODE_A:
		case WIRELESS_MODE_G:
		case WIRELESS_MODE_N_24G:
		case WIRELESS_MODE_N_5G:
			if (pDM_Odm->bNoisyState){
				if (rssi_level==1)
					ret_bitmap&=0x0f0e0c08;
				else if (rssi_level==2)
					ret_bitmap&=0x0f8f0e0c;
				else if (rssi_level==3)
					ret_bitmap&=0x0fefefcc ;
				else
					ret_bitmap&=0xffffffff ;
			}
			else{
				if (rssi_level==1){
					ret_bitmap&=0x0f8f0e08;
				}
				else if (rssi_level==2){
					ret_bitmap&=0x0fcf8f8c;
				}
				else if (rssi_level==3){
					ret_bitmap&=0x0fffffcc;
				}
				else{
					ret_bitmap&=0x0fffffff;
				}
			}
			break;
		default:
			break;
	}
	//DbgPrint("DM_RAMask ====> rssi_LV = %d, BITMAP = %x \n", rssi_level, ret_bitmap);
	return ret_bitmap;

}



void
ODM_UpdateInitRate(
	IN	PDM_ODM_T	pDM_Odm,
	IN	u1Byte		Rate
	)
{
	u1Byte			p = 0;

	ODM_RT_TRACE(pDM_Odm,ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD,("Get C2H Command! Rate=0x%x\n", Rate));
	
	pDM_Odm->TxRate = Rate;
}
