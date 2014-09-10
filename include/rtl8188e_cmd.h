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
#ifndef __RTL8188E_CMD_H__
#define __RTL8188E_CMD_H__

#if 0
enum cmd_msg_element_id
{
	NONE_CMDMSG_EID,
	AP_OFFLOAD_EID = 0,
	SET_PWRMODE_EID = 1,
	JOINBSS_RPT_EID = 2,
	RSVD_PAGE_EID = 3,
	RSSI_4_EID = 4,
	RSSI_SETTING_EID = 5,
	MACID_CONFIG_EID = 6,
	MACID_PS_MODE_EID = 7,
	P2P_PS_OFFLOAD_EID = 8,
	SELECTIVE_SUSPEND_ROF_CMD = 9,
	P2P_PS_CTW_CMD_EID = 32,
	MAX_CMDMSG_EID
};
#else
typedef enum _RTL8188E_H2C_CMD_ID
{
	//Class Common
	H2C_COM_RSVD_PAGE			=0x00,
	H2C_COM_MEDIA_STATUS_RPT	=0x01,
	H2C_COM_SCAN					=0x02,
	H2C_COM_KEEP_ALIVE			=0x03,
	H2C_COM_DISCNT_DECISION		=0x04,
#ifndef CONFIG_WOWLAN
	H2C_COM_WWLAN				=0x05,
#endif
	H2C_COM_INIT_OFFLOAD			=0x06,
	H2C_COM_REMOTE_WAKE_CTL	=0x07,
	H2C_COM_AP_OFFLOAD			=0x08,
	H2C_COM_BCN_RSVD_PAGE		=0x09,
	H2C_COM_PROB_RSP_RSVD_PAGE	=0x0A,

	H2C_COM_GPIO_CTRL				=0x13,
	//Class PS
	H2C_PS_PWR_MODE				=0x20,
	H2C_PS_TUNE_PARA				=0x21,
	H2C_PS_TUNE_PARA_2			=0x22,
	H2C_PS_LPS_PARA				=0x23,
	H2C_PS_P2P_OFFLOAD			=0x24,
#ifdef SOFTAP_PS_DURATION
	H2C_PS_SOFTAP			=0x26,
#endif
	//Class DM
	H2C_DM_MACID_CFG				=0x40,
	H2C_DM_TXBF					=0x41,

	//Class BT
	H2C_BT_COEX_MASK				=0x60,
	H2C_BT_COEX_GPIO_MODE		=0x61,
	H2C_BT_DAC_SWING_VAL			=0x62,
	H2C_BT_PSD_RST				=0x63,

	//Class Remote WakeUp
#ifdef CONFIG_WOWLAN
	H2C_COM_WWLAN				=0x80,
	H2C_COM_REMOTE_WAKE_CTRL	=0x81,
	H2C_COM_AOAC_GLOBAL_INFO	=0x82,
	H2C_COM_AOAC_RSVD_PAGE 		= 0x83,
#endif
	//Class
	 H2C_RESET_TSF				=0xc0,
}RTL8188E_H2C_CMD_ID;

#endif


struct cmd_msg_parm {
	u8 eid; //element id
	u8 sz; // sz
	u8 buf[6];
};

enum{
	PWRS
};

typedef struct _SETPWRMODE_PARM {
	u8 Mode;//0:Active,1:LPS,2:WMMPS
	//u8 RLBM:4;//0:Min,1:Max,2: User define
	u8 SmartPS_RLBM;//LPS=0:PS_Poll,1:PS_Poll,2:NullData,WMM=0:PS_Poll,1:NullData
	u8 AwakeInterval;	// unit: beacon interval
	u8 bAllQueueUAPSD;
	u8 PwrState;//AllON(0x0c),RFON(0x04),RFOFF(0x00)
} SETPWRMODE_PARM, *PSETPWRMODE_PARM;

struct H2C_SS_RFOFF_PARAM{
	u8 ROFOn; // 1: on, 0:off
	u16 gpio_period; // unit: 1024 us
}__attribute__ ((packed));


typedef struct JOINBSSRPT_PARM{
	u8 OpMode;	// RT_MEDIA_STATUS
#ifdef CONFIG_WOWLAN
	u8 MacID;       // MACID
#endif //CONFIG_WOWLAN
}JOINBSSRPT_PARM, *PJOINBSSRPT_PARM;

typedef struct _RSVDPAGE_LOC {
	u8 LocProbeRsp;
	u8 LocPsPoll;
	u8 LocNullData;
	u8 LocQosNull;
	u8 LocBTQosNull;
#ifdef CONFIG_WOWLAN
	u8 LocRemoteCtrlInfo;
	u8 LocArpRsp;
	u8 LocNbrAdv;
	u8 LocGTKRsp;
	u8 LocGTKInfo;
	u8 LocProbeReq;
	u8 LocNetList;
#endif //CONFIG_WOWLAN
} RSVDPAGE_LOC, *PRSVDPAGE_LOC;

struct P2P_PS_Offload_t {
	u8 Offload_En:1;
	u8 role:1; // 1: Owner, 0: Client
	u8 CTWindow_En:1;
	u8 NoA0_En:1;
	u8 NoA1_En:1;
	u8 AllStaSleep:1; // Only valid in Owner
	u8 discovery:1;
	u8 rsvd:1;
};

struct P2P_PS_CTWPeriod_t {
	u8 CTWPeriod;	//TU
};


// host message to firmware cmd
void rtl8188e_set_FwPwrMode_cmd(PADAPTER padapter, u8 Mode);
void rtl8188e_set_FwJoinBssReport_cmd(PADAPTER padapter, u8 mstatus);
u8 rtl8188e_set_rssi_cmd(PADAPTER padapter, u8 *param);
u8 rtl8188e_set_raid_cmd(PADAPTER padapter, u32 mask);
void rtl8188e_Add_RateATid(PADAPTER padapter, u32 bitmap, u8 arg);
//u8 rtl8192c_set_FwSelectSuspend_cmd(PADAPTER padapter, u8 bfwpoll, u16 period);


#ifdef CONFIG_P2P
void rtl8192c_set_p2p_ps_offload_cmd(PADAPTER padapter, u8 p2p_ps_state);
//void rtl8723a_set_p2p_ps_offload_cmd(PADAPTER padapter, u8 p2p_ps_state);
#endif //CONFIG_P2P

void CheckFwRsvdPageContent(PADAPTER padapter);
void rtl8188e_set_FwMediaStatus_cmd(PADAPTER padapter, u16 mstatus_rpt );
void rtl8188e_set_FwPsTuneParam_cmd(PADAPTER padapter);

#ifdef CONFIG_TSF_RESET_OFFLOAD
//u8 rtl8188e_reset_tsf(_adapter *padapter, u8 reset_port);
int reset_tsf(PADAPTER Adapter, u8 reset_port );
#endif	// CONFIG_TSF_RESET_OFFLOAD

#define eqMacAddr(a,b)						( ((a)[0]==(b)[0] && (a)[1]==(b)[1] && (a)[2]==(b)[2] && (a)[3]==(b)[3] && (a)[4]==(b)[4] && (a)[5]==(b)[5]) ? 1:0 )
#define cpMacAddr(des,src)					((des)[0]=(src)[0],(des)[1]=(src)[1],(des)[2]=(src)[2],(des)[3]=(src)[3],(des)[4]=(src)[4],(des)[5]=(src)[5])
#define cpIpAddr(des,src)					((des)[0]=(src)[0],(des)[1]=(src)[1],(des)[2]=(src)[2],(des)[3]=(src)[3])

//
// ARP packet
//
// LLC Header
#define GET_ARP_PKT_LLC_TYPE(__pHeader) 					ReadEF2Byte( ((u8*)(__pHeader)) + 6)

//ARP element
#define GET_ARP_PKT_OPERATION(__pHeader) 				ReadEF2Byte( ((u8*)(__pHeader)) + 6)
#define GET_ARP_PKT_SENDER_MAC_ADDR(__pHeader, _val) 	cpMacAddr((u8*)(_val), ((u8*)(__pHeader))+8)
#define GET_ARP_PKT_SENDER_IP_ADDR(__pHeader, _val) 		cpIpAddr((u8*)(_val), ((u8*)(__pHeader))+14)
#define GET_ARP_PKT_TARGET_MAC_ADDR(__pHeader, _val) 	cpMacAddr((u8*)(_val), ((u8*)(__pHeader))+18)

#define SET_ARP_PKT_HW(__pHeader, __Value)  				WriteEF2Byte( ((u8*)(__pHeader)) + 0, __Value)
#define SET_ARP_PKT_PROTOCOL(__pHeader, __Value)  			WriteEF2Byte( ((u8*)(__pHeader)) + 2, __Value)
#define SET_ARP_PKT_HW_ADDR_LEN(__pHeader, __Value)  		WriteEF1Byte( ((u8*)(__pHeader)) + 4, __Value)
#define SET_ARP_PKT_PROTOCOL_ADDR_LEN(__pHeader, __Value)  	WriteEF1Byte( ((u8*)(__pHeader)) + 5, __Value)
#define SET_ARP_PKT_OPERATION(__pHeader, __Value) 		WriteEF2Byte( ((u8*)(__pHeader)) + 6, __Value)
#define SET_ARP_PKT_SENDER_MAC_ADDR(__pHeader, _val) 	cpMacAddr(((u8*)(__pHeader))+8, (u8*)(_val))
#define SET_ARP_PKT_SENDER_IP_ADDR(__pHeader, _val) 		cpIpAddr(((u8*)(__pHeader))+14, (u8*)(_val))
#define SET_ARP_PKT_TARGET_MAC_ADDR(__pHeader, _val) 	cpMacAddr(((u8*)(__pHeader))+18, (u8*)(_val))
#define SET_ARP_PKT_TARGET_IP_ADDR(__pHeader, _val) 		cpIpAddr(((u8*)(__pHeader))+24, (u8*)(_val))

typedef struct _SETWOWLAN_PARM{
	u8		mode;
	u8		gpio_index;
	u8		gpio_duration;
	u8		second_mode;
	u8		reserve;
}SETWOWLAN_PARM, *PSETWOWLAN_PARM;

typedef struct _SETAOAC_GLOBAL_INFO{
        u8              pairwiseEncAlg;
        u8              groupEncAlg;
}SETAOAC_GLOBAL_INFO, *PSETAOAC_GLOBAL_INFO;

#if 0
#define FW_WOWLAN_FUN_EN				BIT(0)
#define FW_WOWLAN_PATTERN_MATCH			BIT(1)
#define FW_WOWLAN_MAGIC_PKT				BIT(2)
#define FW_WOWLAN_UNICAST				BIT(3)
#define FW_WOWLAN_ALL_PKT_DROP			BIT(4)
#define FW_WOWLAN_GPIO_ACTIVE			BIT(5)
#define FW_WOWLAN_REKEY_WAKEUP			BIT(6)
#define FW_WOWLAN_DEAUTH_WAKEUP			BIT(7)

#define FW_WOWLAN_GPIO_WAKEUP_EN		BIT(0)
#define FW_FW_PARSE_MAGIC_PKT			BIT(1)

#define FW_REMOTE_WAKE_CTRL_EN			BIT(0)
#define FW_REALWOWLAN_EN				BIT(5)
#endif

#define H2C_8188E_RSVDPAGE_LOC_LEN		5
#define H2C_8188E_KEEP_ALIVE_CTRL_LEN	2
#define H2C_8188E_DISCON_DECISION_LEN	 	3
#define H2C_8188E_GPIO_CTRL_LEN	 	3
#define H2C_8188E_PWRMODE_LEN			6
#define H2C_8188E_PSTUNEPARAM_LEN			4
#define H2C_8188E_WOWLAN_LEN			3
#define H2C_8188E_REMOTE_WAKE_CTRL_LEN	1
#define H2C_8188E_AOAC_GLOBAL_INFO_LEN	2
#define H2C_8188E_AOAC_RSVDPAGE_LOC_LEN	7

//_RSVDPAGE_LOC_CMD_0x00
#define SET_8188E_H2CCMD_RSVDPAGE_LOC_PROBE_RSP(__pH2CCmd, __Value)		SET_BITS_TO_LE_1BYTE(__pH2CCmd, 0, 8, __Value)
#define SET_8188E_H2CCMD_RSVDPAGE_LOC_PSPOLL(__pH2CCmd, __Value)			SET_BITS_TO_LE_1BYTE((__pH2CCmd)+1, 0, 8, __Value)
#define SET_8188E_H2CCMD_RSVDPAGE_LOC_NULL_DATA(__pH2CCmd, __Value)		SET_BITS_TO_LE_1BYTE((__pH2CCmd)+2, 0, 8, __Value)
#define SET_8188E_H2CCMD_RSVDPAGE_LOC_QOS_NULL_DATA(__pH2CCmd, __Value)		SET_BITS_TO_LE_1BYTE((__pH2CCmd)+3, 0, 8, __Value)
#define SET_8188E_H2CCMD_RSVDPAGE_LOC_BT_QOS_NULL_DATA(__pH2CCmd, __Value)		SET_BITS_TO_LE_1BYTE((__pH2CCmd)+4, 0, 8, __Value)

//_KEEP_ALIVE_CMD_0x03
#define SET_8188E_H2CCMD_KEEPALIVE_PARM_ENABLE(__pH2CCmd, __Value)		SET_BITS_TO_LE_1BYTE(__pH2CCmd, 0, 1, __Value)
#define SET_8188E_H2CCMD_KEEPALIVE_PARM_ADOPT(__pH2CCmd, __Value)		SET_BITS_TO_LE_1BYTE(__pH2CCmd, 1, 1, __Value)
#define SET_8188E_H2CCMD_KEEPALIVE_PARM_CHECK_PERIOD(__pH2CCmd, __Value)		SET_BITS_TO_LE_1BYTE(__pH2CCmd+1, 0, 8, __Value)

//_DISCONNECT_DECISION_CMD_0x04
#define SET_8188E_H2CCMD_DISCONDECISION_PARM_ENABLE(__pH2CCmd, __Value)		SET_BITS_TO_LE_1BYTE(__pH2CCmd, 0, 1, __Value)
#define SET_8188E_H2CCMD_DISCONDECISION_PARM_ADOPT(__pH2CCmd, __Value)		SET_BITS_TO_LE_1BYTE(__pH2CCmd, 1, 1, __Value)
#define SET_8188E_H2CCMD_DISCONDECISION_PARM_CHECK_PERIOD(__pH2CCmd, __Value)		SET_BITS_TO_LE_1BYTE(__pH2CCmd+1, 0, 8, __Value)
#define SET_8188E_H2CCMD_DISCONDECISION_PARM_TRY_PKT_NUM(__pH2CCmd, __Value)		SET_BITS_TO_LE_1BYTE(__pH2CCmd+2, 0, 8, __Value)

// _GPIO_CTRL_CMD_0x13
#define SET_8188E_H2CCMD_GPIOCTRL_PARM_GPIOIDX(__pH2CCmd, __Value)	SET_BITS_TO_LE_1BYTE((__pH2CCmd)+0, 0, 4, __Value)
#define SET_8188E_H2CCMD_GPIOCTRL_PARM_C2HEN(__pH2CCmd, __Value)	SET_BITS_TO_LE_1BYTE((__pH2CCmd)+0, 4, 1, __Value)
#define SET_8188E_H2CCMD_GPIOCTRL_PARM_PULSE(__pH2CCmd, __Value)	SET_BITS_TO_LE_1BYTE((__pH2CCmd)+0, 5, 1, __Value)
#define SET_8188E_H2CCMD_GPIOCTRL_PARM_HIGH_ACTIVE(__pH2CCmd, __Value)	SET_BITS_TO_LE_1BYTE((__pH2CCmd)+0, 6, 1, __Value)
#define SET_8188E_H2CCMD_GPIOCTRL_PARM_EN(__pH2CCmd, __Value)		SET_BITS_TO_LE_1BYTE((__pH2CCmd)+0, 7, 1, __Value)
#define SET_8188E_H2CCMD_GPIOCTRL_PARM_DUR(__pH2CCmd, __Value)		SET_BITS_TO_LE_1BYTE((__pH2CCmd)+1, 0, 8, __Value)
#define SET_8188E_H2CCMD_GPIOCTRL_PARM_C2HDUR(__pH2CCmd, __Value)	SET_BITS_TO_LE_1BYTE((__pH2CCmd)+2, 0, 8, __Value)

// _PWR_MODE_CMD_0x20
#define SET_8188E_H2CCMD_PWRMODE_PARM_MODE(__pH2CCmd, __Value)			SET_BITS_TO_LE_1BYTE(__pH2CCmd, 0, 8, __Value)
#define SET_8188E_H2CCMD_PWRMODE_PARM_RLBM(__pH2CCmd, __Value)			SET_BITS_TO_LE_1BYTE((__pH2CCmd)+1, 0, 4, __Value)
#define SET_8188E_H2CCMD_PWRMODE_PARM_SMART_PS(__pH2CCmd, __Value)		SET_BITS_TO_LE_1BYTE((__pH2CCmd)+1, 4, 4, __Value)
#define SET_8188E_H2CCMD_PWRMODE_PARM_BCN_PASS_TIME(__pH2CCmd, __Value)	SET_BITS_TO_LE_1BYTE((__pH2CCmd)+2, 0, 8, __Value)
#define SET_8188E_H2CCMD_PWRMODE_PARM_ALL_QUEUE_UAPSD(__pH2CCmd, __Value)	SET_BITS_TO_LE_1BYTE((__pH2CCmd)+3, 0, 8, __Value)
#define SET_8188E_H2CCMD_PWRMODE_PARM_PWR_STATE(__pH2CCmd, __Value)	SET_BITS_TO_LE_1BYTE((__pH2CCmd)+4, 0, 8, __Value)
#define SET_8188E_H2CCMD_PWRMODE_PARM_BYTE5(__pH2CCmd, __Value)	SET_BITS_TO_LE_1BYTE((__pH2CCmd)+5, 0, 8, __Value)

// _PS_TUNE_PARAM_CMD_0x21
#define SET_8188E_H2CCMD_PSTUNE_PARM_BCN_TO_LIMIT(__pH2CCmd, __Value)			SET_BITS_TO_LE_1BYTE(__pH2CCmd, 0, 8, __Value)
#define SET_8188E_H2CCMD_PSTUNE_PARM_DTIM_TIMEOUT(__pH2CCmd, __Value)			SET_BITS_TO_LE_1BYTE(__pH2CCmd+1, 0, 8, __Value)
#define SET_8188E_H2CCMD_PSTUNE_PARM_ADOPT(__pH2CCmd, __Value)			SET_BITS_TO_LE_1BYTE(__pH2CCmd+2, 0, 1, __Value)
#define SET_8188E_H2CCMD_PSTUNE_PARM_PS_TIMEOUT(__pH2CCmd, __Value)			SET_BITS_TO_LE_1BYTE(__pH2CCmd+2, 1, 7, __Value)
#define SET_8188E_H2CCMD_PSTUNE_PARM_DTIM_PERIOD(__pH2CCmd, __Value)			SET_BITS_TO_LE_1BYTE(__pH2CCmd+3, 0, 8, __Value)

// _WoWLAN PARAM_CMD_0x80
#define SET_8188E_H2CCMD_WOWLAN_FUNC_ENABLE(__pH2CCmd, __Value)		SET_BITS_TO_LE_1BYTE(__pH2CCmd, 0, 1, __Value)
#define SET_8188E_H2CCMD_WOWLAN_PATTERN_MATCH_ENABLE(__pH2CCmd, __Value)		SET_BITS_TO_LE_1BYTE(__pH2CCmd, 1, 1, __Value)
#define SET_8188E_H2CCMD_WOWLAN_MAGIC_PKT_ENABLE(__pH2CCmd, __Value)		SET_BITS_TO_LE_1BYTE(__pH2CCmd, 2, 1, __Value)
#define SET_8188E_H2CCMD_WOWLAN_UNICAST_PKT_ENABLE(__pH2CCmd, __Value)		SET_BITS_TO_LE_1BYTE(__pH2CCmd, 3, 1, __Value)
#define SET_8188E_H2CCMD_WOWLAN_ALL_PKT_DROP(__pH2CCmd, __Value)		SET_BITS_TO_LE_1BYTE(__pH2CCmd, 4, 1, __Value)
#define SET_8188E_H2CCMD_WOWLAN_GPIO_ACTIVE(__pH2CCmd, __Value)				SET_BITS_TO_LE_1BYTE(__pH2CCmd, 5, 1, __Value)
#define SET_8188E_H2CCMD_WOWLAN_REKEY_WAKE_UP(__pH2CCmd, __Value)		SET_BITS_TO_LE_1BYTE(__pH2CCmd, 6, 1, __Value)
#define SET_8188E_H2CCMD_WOWLAN_DISCONNECT_WAKE_UP(__pH2CCmd, __Value)		SET_BITS_TO_LE_1BYTE(__pH2CCmd, 7, 1, __Value)
#define SET_8188E_H2CCMD_WOWLAN_GPIONUM(__pH2CCmd, __Value)					SET_BITS_TO_LE_1BYTE((__pH2CCmd)+1, 0, 7, __Value)
#define SET_8188E_H2CCMD_WOWLAN_DATAPIN_WAKE_UP(__pH2CCmd, __Value)					SET_BITS_TO_LE_1BYTE((__pH2CCmd)+1, 7, 1, __Value)
#define SET_8188E_H2CCMD_WOWLAN_GPIO_DURATION(__pH2CCmd, __Value)			SET_BITS_TO_LE_1BYTE((__pH2CCmd)+2, 0, 8, __Value)

// _REMOTE_WAKEUP_CMD_0x81
#define SET_8188E_H2CCMD_REMOTE_WAKECTRL_ENABLE(__pH2CCmd, __Value)		SET_BITS_TO_LE_1BYTE(__pH2CCmd, 0, 1, __Value)
#define SET_8188E_H2CCMD_REMOTE_WAKE_CTRL_ARP_OFFLOAD_EN(__pH2CCmd, __Value)				SET_BITS_TO_LE_1BYTE(__pH2CCmd, 1, 1, __Value)
#define SET_8188E_H2CCMD_REMOTE_WAKE_CTRL_NDP_OFFLOAD_EN(__pH2CCmd, __Value)				SET_BITS_TO_LE_1BYTE(__pH2CCmd, 2, 1, __Value)
#define SET_8188E_H2CCMD_REMOTE_WAKE_CTRL_GTK_OFFLOAD_EN(__pH2CCmd, __Value)				SET_BITS_TO_LE_1BYTE(__pH2CCmd, 3, 1, __Value)
#define SET_8188E_H2CCMD_REMOTE_WAKE_CTRL_SCAN_OFFLOAD_EN(__pH2CCmd, __Value)				SET_BITS_TO_LE_1BYTE(__pH2CCmd, 4, 1, __Value)
#define SET_8188E_H2CCMD_REMOTE_WAKE_CTRL_REAL_WOWLAN_EN(__pH2CCmd, __Value)				SET_BITS_TO_LE_1BYTE(__pH2CCmd, 5, 1, __Value)
#define SET_8188E_H2CCMD_REMOTE_WAKE_CTRL_REAL_WOWLAN_V2_EN(__pH2CCmd, __Value)				SET_BITS_TO_LE_1BYTE(__pH2CCmd, 6, 1, __Value)
#define SET_8188E_H2CCMD_REMOTE_WAKE_CTRL_MATCH_QQ_EN(__pH2CCmd, __Value)				SET_BITS_TO_LE_1BYTE(__pH2CCmd, 7, 1, __Value)

// AOAC_GLOBAL_INFO_0x82
#define SET_8188E_H2CCMD_AOAC_GLOBAL_INFO_PAIRWISE_ENC_ALG(__pH2CCmd, __Value)		SET_BITS_TO_LE_1BYTE(__pH2CCmd, 0, 8, __Value)
#define SET_8188E_H2CCMD_AOAC_GLOBAL_INFO_GROUP_ENC_ALG(__pH2CCmd, __Value)			SET_BITS_TO_LE_1BYTE((__pH2CCmd)+1, 0, 8, __Value)

// AOAC_RSVDPAGE_LOC_0x83
#define SET_8188E_H2CCMD_AOAC_RSVDPAGE_LOC_REMOTE_WAKE_CTRL_INFO(__pH2CCmd, __Value)		SET_BITS_TO_LE_1BYTE((__pH2CCmd), 0, 8, __Value)
#define SET_8188E_H2CCMD_AOAC_RSVDPAGE_LOC_ARP_RSP(__pH2CCmd, __Value)					SET_BITS_TO_LE_1BYTE((__pH2CCmd)+1, 0, 8, __Value)
#define SET_8188E_H2CCMD_AOAC_RSVDPAGE_LOC_NEIGHBOR_ADV(__pH2CCmd, __Value)			SET_BITS_TO_LE_1BYTE((__pH2CCmd)+2, 0, 8, __Value)
#define SET_8188E_H2CCMD_AOAC_RSVDPAGE_LOC_GTK_RSP(__pH2CCmd, __Value)					SET_BITS_TO_LE_1BYTE((__pH2CCmd)+3, 0, 8, __Value)
#define SET_8188E_H2CCMD_AOAC_RSVDPAGE_LOC_GTK_INFO(__pH2CCmd, __Value)					SET_BITS_TO_LE_1BYTE((__pH2CCmd)+4, 0, 8, __Value)
#define SET_8188E_H2CCMD_AOAC_RSVDPAGE_LOC_PROBE_REQ(__pH2CCmd, __Value)					SET_BITS_TO_LE_1BYTE((__pH2CCmd)+5, 0, 8, __Value)
#define SET_8188E_H2CCMD_AOAC_RSVDPAGE_LOC_NETWORK_LIST(__pH2CCmd, __Value)					SET_BITS_TO_LE_1BYTE((__pH2CCmd)+6, 0, 8, __Value)


void rtl8188es_set_wowlan_cmd(_adapter* padapter, u8 enable);
void SetFwRelatedForWoWLAN8188ES(_adapter* padapter, u8 bHostIsGoingtoSleep);

#ifdef SOFTAP_PS_DURATION
typedef struct _SETSOFTAPPS_PARM{
	u8		mode;
	u8		duration;
}SETSOFTAPPS_PARM, *PSETSOFTAPPS_PARM;

void rtl8188es_set_softap_ps_cmd(_adapter* padapter, u8 enable, u8 duration);
#endif //SOFTAP_PS_DURATION
#endif//__RTL8188E_CMD_H__


