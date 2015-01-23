/******************************************************************************
 *
 * Copyright(c) 2007 - 2012 Realtek Corporation. All rights reserved.
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
#define _RTW_PWRCTRL_C_

#include <drv_types.h>
#include <hal_data.h>
#include <linux/jiffies.h>


int rtw_fw_ps_state(PADAPTER padapter)
{
	struct dvobj_priv *psdpriv = padapter->dvobj;
	struct debug_priv *pdbgpriv = &psdpriv->drv_dbg;
	int ret=_FAIL, dont_care=0;
	u16 fw_ps_state=0;
	u32 start_time;
	struct pwrctrl_priv *pwrpriv = adapter_to_pwrctl(padapter);
	struct registry_priv  *registry_par = &padapter->registrypriv;
	
	if(registry_par->check_fw_ps != 1)
		return _SUCCESS;
	
	down(&pwrpriv->check_32k_lock);
	
	if ((padapter->bSurpriseRemoved == true))
	{
		DBG_871X("%s: bSurpriseRemoved=%d , hw_init_completed=%d, bDriverStopped=%d \n", __FUNCTION__, padapter->bSurpriseRemoved,
		padapter->hw_init_completed,padapter->bDriverStopped);
		goto exit_fw_ps_state;
	}
	rtw_hal_set_hwreg(padapter, HW_VAR_SET_REQ_FW_PS, (u8 *)&dont_care);
	{
		//4. if 0x88[7]=1, driver set cmd to leave LPS/IPS. 
		//Else, hw will keep in active mode.
		//debug info:
		//0x88[7] = 32kpermission, 
		//0x88[6:0] = current_ps_state
		//0x89[7:0] = last_rpwm

		rtw_hal_get_hwreg(padapter, HW_VAR_FW_PS_STATE, (u8 *)&fw_ps_state);
		
		if((fw_ps_state & 0x80) == 0)
			ret=_SUCCESS;
		else
		{
			pdbgpriv->dbg_poll_fail_cnt++;
			DBG_871X("%s: fw_ps_state=%04x \n", __FUNCTION__, fw_ps_state);
		}
	}


exit_fw_ps_state:
	up(&pwrpriv->check_32k_lock);
	return ret;
}

void _ips_enter(_adapter * padapter)
{
	struct pwrctrl_priv *pwrpriv = adapter_to_pwrctl(padapter);

	pwrpriv->bips_processing = true;	

	// syn ips_mode with request
	pwrpriv->ips_mode = pwrpriv->ips_mode_req;
	
	pwrpriv->ips_enter_cnts++;	
	DBG_871X("==>ips_enter cnts:%d\n",pwrpriv->ips_enter_cnts);

	if(rf_off == pwrpriv->change_rfpwrstate )
	{	
		pwrpriv->bpower_saving = true;
		DBG_871X_LEVEL(_drv_always_, "nolinked power save enter\n");

		if(pwrpriv->ips_mode == IPS_LEVEL_2)
			pwrpriv->bkeepfwalive = true;
		
		rtw_ips_pwr_down(padapter);
		pwrpriv->rf_pwrstate = rf_off;
	}	
	pwrpriv->bips_processing = false;	
	
}

void ips_enter(_adapter * padapter)
{
	struct pwrctrl_priv *pwrpriv = adapter_to_pwrctl(padapter);


	rtw_btcoex_IpsNotify(padapter, pwrpriv->ips_mode_req);

	down(&pwrpriv->lock);
	_ips_enter(padapter);
	up(&pwrpriv->lock);
}

int _ips_leave(_adapter * padapter)
{
	struct pwrctrl_priv *pwrpriv = adapter_to_pwrctl(padapter);
	int result = _SUCCESS;

	if((pwrpriv->rf_pwrstate == rf_off) &&(!pwrpriv->bips_processing))
	{
		pwrpriv->bips_processing = true;
		pwrpriv->change_rfpwrstate = rf_on;
		pwrpriv->ips_leave_cnts++;
		DBG_871X("==>ips_leave cnts:%d\n",pwrpriv->ips_leave_cnts);

		if ((result = rtw_ips_pwr_up(padapter)) == _SUCCESS) {
			pwrpriv->rf_pwrstate = rf_on;
		}
		DBG_871X_LEVEL(_drv_always_, "nolinked power save leave\n");
		
		DBG_871X("==> ips_leave.....LED(0x%08x)...\n",rtw_read32(padapter,0x4c));
		pwrpriv->bips_processing = false;

		pwrpriv->bkeepfwalive = false;
		pwrpriv->bpower_saving = false;
	}

	return result;
}

int ips_leave(_adapter * padapter)
{
	struct pwrctrl_priv *pwrpriv = adapter_to_pwrctl(padapter);
	struct dvobj_priv *psdpriv = padapter->dvobj;
	struct debug_priv *pdbgpriv = &psdpriv->drv_dbg;
	int ret;

	if(!is_primary_adapter(padapter))
		return _SUCCESS;

	down(&pwrpriv->lock);
	ret = _ips_leave(padapter);
	up(&pwrpriv->lock);

	if (_SUCCESS == ret)
		ODM_DMReset(&GET_HAL_DATA(padapter)->odmpriv);

	if (_SUCCESS == ret)
		rtw_btcoex_IpsNotify(padapter, IPS_NONE);

	return ret;
}

static bool rtw_pwr_unassociated_idle(_adapter *adapter)
{
	_adapter *buddy = adapter->pbuddy_adapter;
	struct mlme_priv *pmlmepriv = &(adapter->mlmepriv);
	struct xmit_priv *pxmit_priv = &adapter->xmitpriv;

	bool ret = false;

	if (adapter_to_pwrctl(adapter)->bpower_saving ==true ) {
		//DBG_871X("%s: already in LPS or IPS mode\n", __func__);
		goto exit;
	}

	if (time_before(jiffies, adapter_to_pwrctl(adapter)->ips_deny_time)) {
		//DBG_871X("%s ips_deny_time\n", __func__);
		goto exit;
	}

	if (check_fwstate(pmlmepriv, WIFI_ASOC_STATE|WIFI_SITE_MONITOR)
		|| check_fwstate(pmlmepriv, WIFI_UNDER_LINKING|WIFI_UNDER_WPS)
		|| check_fwstate(pmlmepriv, WIFI_AP_STATE)
		|| check_fwstate(pmlmepriv, WIFI_ADHOC_MASTER_STATE|WIFI_ADHOC_STATE)
	) {
		goto exit;
	}

	/* consider buddy, if exist */
	if (buddy) {
		struct mlme_priv *b_pmlmepriv = &(buddy->mlmepriv);

		if (check_fwstate(b_pmlmepriv, WIFI_ASOC_STATE|WIFI_SITE_MONITOR)
			|| check_fwstate(b_pmlmepriv, WIFI_UNDER_LINKING|WIFI_UNDER_WPS)
			|| check_fwstate(b_pmlmepriv, WIFI_AP_STATE)
			|| check_fwstate(b_pmlmepriv, WIFI_ADHOC_MASTER_STATE|WIFI_ADHOC_STATE)
		) {
			goto exit;
		}
	}

	if (pxmit_priv->free_xmitbuf_cnt != NR_XMITBUFF ||
		pxmit_priv->free_xmit_extbuf_cnt != NR_XMIT_EXTBUFF) {
		DBG_871X_LEVEL(_drv_always_, "There are some pkts to transmit\n");
		DBG_871X_LEVEL(_drv_always_, "free_xmitbuf_cnt: %d, free_xmit_extbuf_cnt: %d\n", 
			pxmit_priv->free_xmitbuf_cnt, pxmit_priv->free_xmit_extbuf_cnt);	
		goto exit;
	}

	ret = true;

exit:
	return ret;
}


/*
 * ATTENTION:
 *	rtw_ps_processor() doesn't handle LPS.
 */
void rtw_ps_processor(_adapter*padapter)
{
	struct pwrctrl_priv *pwrpriv = adapter_to_pwrctl(padapter);
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	struct dvobj_priv *psdpriv = padapter->dvobj;
	struct debug_priv *pdbgpriv = &psdpriv->drv_dbg;
	u32 ps_deny = 0;

	down(&adapter_to_pwrctl(padapter)->lock);
	ps_deny = rtw_ps_deny_get(padapter);
	up(&adapter_to_pwrctl(padapter)->lock);
	if (ps_deny != 0)
	{
		DBG_871X(FUNC_ADPT_FMT ": ps_deny=0x%08X, skip power save!\n",
			FUNC_ADPT_ARG(padapter), ps_deny);
		goto exit;
	}

	if(pwrpriv->bInSuspend == true){//system suspend or autosuspend
		pdbgpriv->dbg_ps_insuspend_cnt++;
		DBG_871X("%s, pwrpriv->bInSuspend == true ignore this process\n",__FUNCTION__);
		return;
	}	

	pwrpriv->ps_processing = true;

	if (pwrpriv->ips_mode_req == IPS_NONE)
		goto exit;

	if (rtw_pwr_unassociated_idle(padapter) == false)
		goto exit;

	if((pwrpriv->rf_pwrstate == rf_on) && ((pwrpriv->pwr_state_check_cnts%4)==0))
	{
		DBG_871X("==>%s .fw_state(%x)\n",__FUNCTION__,get_fwstate(pmlmepriv));
		pwrpriv->change_rfpwrstate = rf_off;
		{
			ips_enter(padapter);			
		}
	}
exit:
	pwrpriv->ps_processing = false;
	return;
}

void pwr_state_check_handler(RTW_TIMER_HDL_ARGS);
void pwr_state_check_handler(RTW_TIMER_HDL_ARGS)
{
	_adapter *padapter = (_adapter *)FunctionContext;
	rtw_ps_cmd(padapter);
}

void	traffic_check_for_leave_lps(PADAPTER padapter, u8 tx, u32 tx_packets)
{
	static unsigned long start_time = 0;
	static u32 xmit_cnt = 0;
	u8	bLeaveLPS = false;
	struct mlme_priv	*pmlmepriv = &padapter->mlmepriv;

	

	if(tx) //from tx
	{
		xmit_cnt += tx_packets;

		if (start_time== 0)
			start_time= jiffies;

		if (jiffies_to_msecs(jiffies - start_time) > 2000) // 2 sec == watch dog timer
		{		
			if(xmit_cnt > 8)
			{
				if ((adapter_to_pwrctl(padapter)->bLeisurePs) 
					&& (adapter_to_pwrctl(padapter)->pwr_mode != PS_MODE_ACTIVE)
					&& (rtw_btcoex_IsBtControlLps(padapter) == false)
					)
				{
					DBG_871X("leave lps via Tx = %d\n", xmit_cnt);			
					bLeaveLPS = true;
				}
			}

			start_time= jiffies;
			xmit_cnt = 0;
		}

	}
	else // from rx path
	{
		if(pmlmepriv->LinkDetectInfo.NumRxUnicastOkInPeriod > 4/*2*/)
		{
			if ((adapter_to_pwrctl(padapter)->bLeisurePs)
				&& (adapter_to_pwrctl(padapter)->pwr_mode != PS_MODE_ACTIVE)
				&& (rtw_btcoex_IsBtControlLps(padapter) == false)
				)
			{	
				DBG_871X("leave lps via Rx = %d\n", pmlmepriv->LinkDetectInfo.NumRxUnicastOkInPeriod);	
				bLeaveLPS = true;
			}
		}	
	}	

	if(bLeaveLPS)
	{
		//DBG_871X("leave lps via %s, Tx = %d, Rx = %d \n", tx?"Tx":"Rx", pmlmepriv->LinkDetectInfo.NumTxOkInPeriod,pmlmepriv->LinkDetectInfo.NumRxUnicastOkInPeriod);	
		//rtw_lps_ctrl_wk_cmd(padapter, LPS_CTRL_LEAVE, 1);
		rtw_lps_ctrl_wk_cmd(padapter, LPS_CTRL_LEAVE, tx?0:1);
	}
}		

/*
 * Description:
 *	This function MUST be called under power lock protect
 *
 * Parameters
 *	padapter
 *	pslv			power state level, only could be PS_STATE_S0 ~ PS_STATE_S4
 *
 */
void rtw_set_rpwm(PADAPTER padapter, u8 pslv)
{
	u8	rpwm;
	struct pwrctrl_priv *pwrpriv = adapter_to_pwrctl(padapter);
	u8 cpwm_orig;
	struct dvobj_priv *psdpriv = padapter->dvobj;
	struct debug_priv *pdbgpriv = &psdpriv->drv_dbg;
_func_enter_;

	pslv = PS_STATE(pslv);

	if (pwrpriv->brpwmtimeout == true)
	{
		DBG_871X("%s: RPWM timeout, force to set RPWM(0x%02X) again!\n", __FUNCTION__, pslv);
	}
	else
	{
		if ( (pwrpriv->rpwm == pslv)
			|| ((pwrpriv->rpwm >= PS_STATE_S2)&&(pslv >= PS_STATE_S2)))
		{
			RT_TRACE(_module_rtl871x_pwrctrl_c_,_drv_err_,
				("%s: Already set rpwm[0x%02X], new=0x%02X!\n", __FUNCTION__, pwrpriv->rpwm, pslv));
			return;
		}
	}

	if ((padapter->bSurpriseRemoved == true) ||
		(padapter->hw_init_completed == false))
	{
		RT_TRACE(_module_rtl871x_pwrctrl_c_, _drv_err_,
				 ("%s: SurpriseRemoved(%d) hw_init_completed(%d)\n",
				  __FUNCTION__, padapter->bSurpriseRemoved, padapter->hw_init_completed));

		pwrpriv->cpwm = PS_STATE_S4;

		return;
	}

	if (padapter->bDriverStopped == true)
	{
		RT_TRACE(_module_rtl871x_pwrctrl_c_, _drv_err_,
				 ("%s: change power state(0x%02X) when DriverStopped\n", __FUNCTION__, pslv));

		if (pslv < PS_STATE_S2) {
			RT_TRACE(_module_rtl871x_pwrctrl_c_, _drv_err_,
					 ("%s: Reject to enter PS_STATE(0x%02X) lower than S2 when DriverStopped!!\n", __FUNCTION__, pslv));
			return;
		}
	}

	rpwm = pslv | pwrpriv->tog;
	// only when from PS_STATE S0/S1 to S2 and higher needs ACK
	if ((pwrpriv->cpwm < PS_STATE_S2) && (pslv >= PS_STATE_S2))
		rpwm |= PS_ACK;
	RT_TRACE(_module_rtl871x_pwrctrl_c_, _drv_notice_,
			 ("rtw_set_rpwm: rpwm=0x%02x cpwm=0x%02x\n", rpwm, pwrpriv->cpwm));

	pwrpriv->rpwm = pslv;

	cpwm_orig = 0;
	if (rpwm & PS_ACK)
	{
		rtw_hal_get_hwreg(padapter, HW_VAR_CPWM, &cpwm_orig);
	}

	if (rpwm & PS_ACK)
		_set_timer(&pwrpriv->pwr_rpwm_timer, LPS_RPWM_WAIT_MS);
	rtw_hal_set_hwreg(padapter, HW_VAR_SET_RPWM, (u8 *)(&rpwm));

	pwrpriv->tog += 0x80;

	// No LPS 32K, No Ack
	if (rpwm & PS_ACK)
	{
		unsigned long start_time;
		u8 cpwm_now;
		u8 poll_cnt=0;

		start_time = jiffies;

		// polling cpwm
		do {
			mdelay(1);
			poll_cnt++;
			rtw_hal_get_hwreg(padapter, HW_VAR_CPWM, &cpwm_now);
			if ((cpwm_orig ^ cpwm_now) & 0x80)
			{
				pwrpriv->cpwm = PS_STATE_S4;
				pwrpriv->cpwm_tog = cpwm_now & PS_TOGGLE;
				break;
			}

			if (jiffies_to_msecs(jiffies - start_time) > LPS_RPWM_WAIT_MS)
			{
				DBG_871X("%s: polling cpwm timeout! poll_cnt=%d, cpwm_orig=%02x, cpwm_now=%02x \n", __FUNCTION__,poll_cnt, cpwm_orig, cpwm_now);
				_set_timer(&pwrpriv->pwr_rpwm_timer, 1);
				break;
			}
		} while (1);
	}
	else
	{
		pwrpriv->cpwm = pslv;
	}

_func_exit_;
}

static u8 PS_RDY_CHECK(_adapter * padapter)
{
	unsigned long curr_time, delta_time;
	struct pwrctrl_priv	*pwrpriv = adapter_to_pwrctl(padapter);
	struct mlme_priv	*pmlmepriv = &(padapter->mlmepriv);

#if defined(CONFIG_WOWLAN) || defined(CONFIG_AP_WOWLAN)
	if(true == pwrpriv->bInSuspend && pwrpriv->wowlan_mode)
		return true;
	else if(true == pwrpriv->bInSuspend && pwrpriv->wowlan_ap_mode)
		return true;
	else if (true == pwrpriv->bInSuspend)
		return false;
#else
	if(true == pwrpriv->bInSuspend )
		return false;
#endif

	curr_time = jiffies;

	delta_time = curr_time -pwrpriv->DelayLPSLastTimeStamp;

	if(delta_time < LPS_DELAY_TIME)
	{		
		return false;
	}

	if (check_fwstate(pmlmepriv, WIFI_SITE_MONITOR)
		|| check_fwstate(pmlmepriv, WIFI_UNDER_LINKING|WIFI_UNDER_WPS)
		|| check_fwstate(pmlmepriv, WIFI_AP_STATE)
		|| check_fwstate(pmlmepriv, WIFI_ADHOC_MASTER_STATE|WIFI_ADHOC_STATE)
		|| rtw_is_scan_deny(padapter)
	)
		return false;

	if( (padapter->securitypriv.dot11AuthAlgrthm == dot11AuthAlgrthm_8021X) && (padapter->securitypriv.binstallGrpkey == false) )
	{
		DBG_871X("Group handshake still in progress !!!\n");
		return false;
	}

	if (!rtw_cfg80211_pwr_mgmt(padapter))
		return false;

	return true;
}

void rtw_set_ps_mode(PADAPTER padapter, u8 ps_mode, u8 smart_ps, u8 bcn_ant_mode, const char *msg)
{
	struct pwrctrl_priv *pwrpriv = adapter_to_pwrctl(padapter);
	struct dvobj_priv *psdpriv = padapter->dvobj;
	struct debug_priv *pdbgpriv = &psdpriv->drv_dbg;

_func_enter_;

	RT_TRACE(_module_rtl871x_pwrctrl_c_, _drv_notice_,
			 ("%s: PowerMode=%d Smart_PS=%d\n",
			  __FUNCTION__, ps_mode, smart_ps));

	if(ps_mode > PM_Card_Disable) {
		RT_TRACE(_module_rtl871x_pwrctrl_c_,_drv_err_,("ps_mode:%d error\n", ps_mode));
		return;
	}

	if (pwrpriv->pwr_mode == ps_mode)
	{
		if (PS_MODE_ACTIVE == ps_mode) return;
	}

	down(&pwrpriv->lock);

	//if(pwrpriv->pwr_mode == PS_MODE_ACTIVE)
	if(ps_mode == PS_MODE_ACTIVE)
	{
		if (1
			&& (((rtw_btcoex_IsBtControlLps(padapter) == false)
					)
				|| ((rtw_btcoex_IsBtControlLps(padapter) == true)
					&& (rtw_btcoex_IsLpsOn(padapter) == false))
				)
			)
		{
			DBG_871X(FUNC_ADPT_FMT" Leave 802.11 power save - %s\n",
				FUNC_ADPT_ARG(padapter), msg);

			pwrpriv->pwr_mode = ps_mode;
			rtw_set_rpwm(padapter, PS_STATE_S4);
			
#if defined(CONFIG_WOWLAN) || defined(CONFIG_AP_WOWLAN)
			if (pwrpriv->wowlan_mode == true ||
					pwrpriv->wowlan_ap_mode == true)
			{
				unsigned long start_time;
				u32 delay_ms;
				u8 val8;
				delay_ms = 20;
				start_time = jiffies;
				do { 
					rtw_hal_get_hwreg(padapter, HW_VAR_SYS_CLKR, &val8);
					if (!(val8 & BIT(4))){ //0x08 bit4 =1 --> in 32k, bit4 = 0 --> leave 32k
						pwrpriv->cpwm = PS_STATE_S4;
						break;
					}
					if (jiffies_to_msecs(jiffies - start_time) > delay_ms)
					{
						DBG_871X("%s: Wait for FW 32K leave more than %u ms!!!\n", 
								__FUNCTION__, delay_ms);
						pdbgpriv->dbg_wow_leave_ps_fail_cnt++;
						break;
					}
					msleep(1);
				} while (1); 
			}
#endif
			rtw_hal_set_hwreg(padapter, HW_VAR_H2C_FW_PWRMODE, (u8 *)(&ps_mode));
			pwrpriv->bFwCurrentInPSMode = false;

			rtw_btcoex_LpsNotify(padapter, ps_mode);
		}
	}
	else
	{
		if ((PS_RDY_CHECK(padapter) && check_fwstate(&padapter->mlmepriv, WIFI_ASOC_STATE))
			|| ((rtw_btcoex_IsBtControlLps(padapter) == true)
				&& (rtw_btcoex_IsLpsOn(padapter) == true))
			)
		{
			u8 pslv;

			DBG_871X(FUNC_ADPT_FMT" Enter 802.11 power save - %s\n",
				FUNC_ADPT_ARG(padapter), msg);

			rtw_btcoex_LpsNotify(padapter, ps_mode);

			pwrpriv->bFwCurrentInPSMode = true;
			pwrpriv->pwr_mode = ps_mode;
			pwrpriv->smart_ps = smart_ps;
			pwrpriv->bcn_ant_mode = bcn_ant_mode;
			rtw_hal_set_hwreg(padapter, HW_VAR_H2C_FW_PWRMODE, (u8 *)(&ps_mode));

			pslv = PS_STATE_S2;
			if (pwrpriv->alives == 0)
				pslv = PS_STATE_S0;

			if ((rtw_btcoex_IsBtDisabled(padapter) == false)
				&& (rtw_btcoex_IsBtControlLps(padapter) == true))
			{
				u8 val8;

				val8 = rtw_btcoex_LpsVal(padapter);
				if (val8 & BIT(4))
					pslv = PS_STATE_S2;
			}

			rtw_set_rpwm(padapter, pslv);
		}
	}

	up(&pwrpriv->lock);

_func_exit_;
}

/*
 * Return:
 *	0:	Leave OK
 *	-1:	Timeout
 *	-2:	Other error
 */
s32 LPS_RF_ON_check(PADAPTER padapter, u32 delay_ms)
{
	unsigned long start_time;
	u8 bAwake = false;
	s32 err = 0;


	start_time = jiffies;
	while (1)
	{
		rtw_hal_get_hwreg(padapter, HW_VAR_FWLPS_RF_ON, &bAwake);
		if (true == bAwake)
			break;

		if (true == padapter->bSurpriseRemoved)
		{
			err = -2;
			DBG_871X("%s: device surprise removed!!\n", __FUNCTION__);
			break;
		}

		if (jiffies_to_msecs(jiffies - start_time) > delay_ms)
		{
			err = -1;
			DBG_871X("%s: Wait for FW LPS leave more than %u ms!!!\n", __FUNCTION__, delay_ms);
			break;
		}
		msleep(1);
	}

	return err;
}

//
//	Description:
//		Enter the leisure power save mode.
//
void LPS_Enter(PADAPTER padapter, const char *msg)
{
	struct dvobj_priv *dvobj = adapter_to_dvobj(padapter);
	struct pwrctrl_priv	*pwrpriv = dvobj_to_pwrctl(dvobj);
	struct mlme_priv	*pmlmepriv = &(padapter->mlmepriv);
	_adapter *buddy = padapter->pbuddy_adapter;
	int n_assoc_iface = 0;
	int i;
	char buf[32] = {0};

_func_enter_;

//	DBG_871X("+LeisurePSEnter\n");

	if (rtw_btcoex_IsBtControlLps(padapter) == true)
		return;

	/* Skip lps enter request if number of assocated adapters is not 1 */
	for (i = 0; i < dvobj->iface_nums; i++) {
		if (check_fwstate(&(dvobj->padapters[i]->mlmepriv), WIFI_ASOC_STATE))
			n_assoc_iface++;
	}
	if (n_assoc_iface != 1)
		return;

	/* Skip lps enter request for adapter not port0 */
	if (get_iface_type(padapter) != IFACE_PORT0)
		return;

	for (i = 0; i < dvobj->iface_nums; i++) {
		if (PS_RDY_CHECK(dvobj->padapters[i]) == false)
			return;
	}

	if (pwrpriv->bLeisurePs)
	{
		// Idle for a while if we connect to AP a while ago.
		if (pwrpriv->LpsIdleCount >= 2) //  4 Sec
		{
			if(pwrpriv->pwr_mode == PS_MODE_ACTIVE)
			{
				sprintf(buf, "WIFI-%s", msg);
				pwrpriv->bpower_saving = true;
				rtw_set_ps_mode(padapter, pwrpriv->power_mgnt, padapter->registrypriv.smart_ps, 0, buf);
			}
		}
		else
			pwrpriv->LpsIdleCount++;
	}

//	DBG_871X("-LeisurePSEnter\n");

_func_exit_;
}

//
//	Description:
//		Leave the leisure power save mode.
//
void LPS_Leave(PADAPTER padapter, const char *msg)
{
#define LPS_LEAVE_TIMEOUT_MS 100

	struct dvobj_priv *dvobj = adapter_to_dvobj(padapter);
	struct pwrctrl_priv	*pwrpriv = dvobj_to_pwrctl(dvobj);
	u32 start_time;
	u8 bAwake = false;
	char buf[32] = {0};
	struct debug_priv *pdbgpriv = &dvobj->drv_dbg;

_func_enter_;

//	DBG_871X("+LeisurePSLeave\n");

	if (rtw_btcoex_IsBtControlLps(padapter) == true)
		return;

	if (pwrpriv->bLeisurePs)
	{
		if(pwrpriv->pwr_mode != PS_MODE_ACTIVE)
		{
			sprintf(buf, "WIFI-%s", msg);
			rtw_set_ps_mode(padapter, PS_MODE_ACTIVE, 0, 0, buf);

			if(pwrpriv->pwr_mode == PS_MODE_ACTIVE)
				LPS_RF_ON_check(padapter, LPS_LEAVE_TIMEOUT_MS);
		}
	}

	pwrpriv->bpower_saving = false;
//	DBG_871X("-LeisurePSLeave\n");

_func_exit_;
}

void LeaveAllPowerSaveModeDirect(PADAPTER Adapter)
{
	PADAPTER pri_padapter = GET_PRIMARY_ADAPTER(Adapter);
	struct mlme_priv	*pmlmepriv = &(Adapter->mlmepriv);
	struct pwrctrl_priv *pwrpriv = adapter_to_pwrctl(Adapter);
	struct dvobj_priv *psdpriv = Adapter->dvobj;
	struct debug_priv *pdbgpriv = &psdpriv->drv_dbg;
	u8 cpwm_orig, cpwm_now;
	unsigned long start_time;

_func_enter_;

	DBG_871X("%s.....\n",__FUNCTION__);

	if (true == Adapter->bSurpriseRemoved)
	{
		DBG_871X(FUNC_ADPT_FMT ": bSurpriseRemoved=%d Skip!\n",
			FUNC_ADPT_ARG(Adapter), Adapter->bSurpriseRemoved);
		return;
	}

	if ((check_fwstate(pmlmepriv, _FW_LINKED) == true))
	{ //connect

		if(pwrpriv->pwr_mode == PS_MODE_ACTIVE) {
			DBG_871X("%s: Driver Already Leave LPS\n",__FUNCTION__);
			return;
		}

		down(&pwrpriv->lock);

		rtw_set_rpwm(Adapter, PS_STATE_S4);

	up(&pwrpriv->lock);

		rtw_lps_ctrl_wk_cmd(pri_padapter, LPS_CTRL_LEAVE, 0);
	}
	else
	{
		if(pwrpriv->rf_pwrstate== rf_off)
		{
			if(false == ips_leave(pri_padapter))
			{
				DBG_871X("======> ips_leave fail.............\n");
			}
		}
	}

_func_exit_;
}

//
// Description: Leave all power save mode: LPS, FwLPS, IPS if needed.
// Move code to function by tynli. 2010.03.26. 
//
void LeaveAllPowerSaveMode(IN PADAPTER Adapter)
{
	struct dvobj_priv *dvobj = adapter_to_dvobj(Adapter);
	struct mlme_priv	*pmlmepriv = &(Adapter->mlmepriv);
	u8	enqueue = 0;
	int n_assoc_iface = 0;
	int i;

_func_enter_;

	//DBG_871X("%s.....\n",__FUNCTION__);

	if (false == Adapter->bup)
	{
		DBG_871X(FUNC_ADPT_FMT ": bup=%d Skip!\n",
			FUNC_ADPT_ARG(Adapter), Adapter->bup);
		return;
	}

	if (true == Adapter->bSurpriseRemoved)
	{
		DBG_871X(FUNC_ADPT_FMT ": bSurpriseRemoved=%d Skip!\n",
			FUNC_ADPT_ARG(Adapter), Adapter->bSurpriseRemoved);
		return;
	}

	for (i = 0; i < dvobj->iface_nums; i++) {
		if (check_fwstate(&(dvobj->padapters[i]->mlmepriv), WIFI_ASOC_STATE))
			n_assoc_iface++;
	}

	if (n_assoc_iface)
	{ //connect
		enqueue = 1;

		rtw_lps_ctrl_wk_cmd(Adapter, LPS_CTRL_LEAVE, enqueue);

		LPS_Leave_check(Adapter);
	}
	else
	{
		if(adapter_to_pwrctl(Adapter)->rf_pwrstate== rf_off)
		{
			if(false == ips_leave(Adapter))
			{
				DBG_871X("======> ips_leave fail.............\n");
			}
		}	
	}

_func_exit_;
}

void LPS_Leave_check(
	PADAPTER padapter)
{
	struct pwrctrl_priv *pwrpriv;
	unsigned long	start_time;
	u8	bReady;

_func_enter_;

	pwrpriv = adapter_to_pwrctl(padapter);

	bReady = false;
	start_time = jiffies;

	yield();
	
	while(1)
	{
		down(&pwrpriv->lock);

		if ((padapter->bSurpriseRemoved == true)
			|| (padapter->hw_init_completed == false)
			|| (pwrpriv->pwr_mode == PS_MODE_ACTIVE)
			)
		{
			bReady = true;
		}

		up(&pwrpriv->lock);

		if(true == bReady)
			break;

		if(jiffies_to_msecs(jiffies - start_time)>100)
		{
			DBG_871X("Wait for cpwm event  than 100 ms!!!\n");
			break;
		}
		msleep(1);
	}

_func_exit_;
}

/*
 * Caller:ISR handler...
 *
 * This will be called when CPWM interrupt is up.
 *
 * using to update cpwn of drv; and drv willl make a decision to up or down pwr level
 */
void cpwm_int_hdl(
	PADAPTER padapter,
	struct reportpwrstate_parm *preportpwrstate)
{
	struct pwrctrl_priv *pwrpriv;

_func_enter_;

	pwrpriv = adapter_to_pwrctl(padapter);

	down(&pwrpriv->lock);

	if (pwrpriv->rpwm < PS_STATE_S2)
	{
		DBG_871X("%s: Redundant CPWM Int. RPWM=0x%02X CPWM=0x%02x\n", __func__, pwrpriv->rpwm, pwrpriv->cpwm);
		up(&pwrpriv->lock);
		goto exit;
	}

	pwrpriv->cpwm = PS_STATE(preportpwrstate->state);
	pwrpriv->cpwm_tog = preportpwrstate->state & PS_TOGGLE;

	if (pwrpriv->cpwm >= PS_STATE_S2)
	{
		if (pwrpriv->alives & CMD_ALIVE)
			up(&padapter->cmdpriv.cmd_queue_sema);

		if (pwrpriv->alives & XMIT_ALIVE)
			up(&padapter->xmitpriv.xmit_sema);
	}

	up(&pwrpriv->lock);

exit:
	RT_TRACE(_module_rtl871x_pwrctrl_c_, _drv_notice_,
			 ("cpwm_int_hdl: cpwm=0x%02x\n", pwrpriv->cpwm));

_func_exit_;
}

static void cpwm_event_callback(struct work_struct *work)
{
	struct pwrctrl_priv *pwrpriv = container_of(work, struct pwrctrl_priv, cpwm_event);
	struct dvobj_priv *dvobj = pwrctl_to_dvobj(pwrpriv);
	_adapter *adapter = dvobj->if1;
	struct reportpwrstate_parm report;

	//DBG_871X("%s\n",__FUNCTION__);

	report.state = PS_STATE_S2;
	cpwm_int_hdl(adapter, &report);
}

static void rpwmtimeout_workitem_callback(struct work_struct *work)
{
	PADAPTER padapter;
	struct dvobj_priv *dvobj;
	struct pwrctrl_priv *pwrpriv;


	pwrpriv = container_of(work, struct pwrctrl_priv, rpwmtimeoutwi);
	dvobj = pwrctl_to_dvobj(pwrpriv);
	padapter = dvobj->if1;
//	DBG_871X("+%s: rpwm=0x%02X cpwm=0x%02X\n", __func__, pwrpriv->rpwm, pwrpriv->cpwm);

	down(&pwrpriv->lock);
	if ((pwrpriv->rpwm == pwrpriv->cpwm) || (pwrpriv->cpwm >= PS_STATE_S2))
	{
		DBG_871X("%s: rpwm=0x%02X cpwm=0x%02X CPWM done!\n", __func__, pwrpriv->rpwm, pwrpriv->cpwm);
		goto exit;
	}
	up(&pwrpriv->lock);

	if (rtw_read8(padapter, 0x100) != 0xEA)
	{
		struct reportpwrstate_parm report;

		report.state = PS_STATE_S2;
		DBG_871X("\n%s: FW already leave 32K!\n\n", __func__);
		cpwm_int_hdl(padapter, &report);

		return;
	}

	down(&pwrpriv->lock);

	if ((pwrpriv->rpwm == pwrpriv->cpwm) || (pwrpriv->cpwm >= PS_STATE_S2))
	{
		DBG_871X("%s: cpwm=%d, nothing to do!\n", __func__, pwrpriv->cpwm);
		goto exit;
	}
	pwrpriv->brpwmtimeout = true;
	rtw_set_rpwm(padapter, pwrpriv->rpwm);
	pwrpriv->brpwmtimeout = false;

exit:
	up(&pwrpriv->lock);
}

/*
 * This function is a timer handler, can't do any IO in it.
 */
static void pwr_rpwm_timeout_handler(void *FunctionContext)
{
	PADAPTER padapter;
	struct pwrctrl_priv *pwrpriv;


	padapter = (PADAPTER)FunctionContext;
	pwrpriv = adapter_to_pwrctl(padapter);
	DBG_871X("+%s: rpwm=0x%02X cpwm=0x%02X\n", __func__, pwrpriv->rpwm, pwrpriv->cpwm);

	if ((pwrpriv->rpwm == pwrpriv->cpwm) || (pwrpriv->cpwm >= PS_STATE_S2))
	{
		DBG_871X("+%s: cpwm=%d, nothing to do!\n", __func__, pwrpriv->cpwm);
		return;
	}

	_set_workitem(&pwrpriv->rpwmtimeoutwi);
}

__inline static void register_task_alive(struct pwrctrl_priv *pwrctrl, u32 tag)
{
	pwrctrl->alives |= tag;
}

__inline static void unregister_task_alive(struct pwrctrl_priv *pwrctrl, u32 tag)
{
	pwrctrl->alives &= ~tag;
}


/*
 * Description:
 *	Check if the fw_pwrstate is okay for I/O.
 *	If not (cpwm is less than S2), then the sub-routine
 *	will raise the cpwm to be greater than or equal to S2.
 *
 *	Calling Context: Passive
 *
 *	Constraint:
 *		1. this function will request pwrctrl->lock
 * 
 * Return Value:
 *	_SUCCESS	hardware is ready for I/O
 *	_FAIL		can't I/O right now
 */
s32 rtw_register_task_alive(PADAPTER padapter, u32 task)
{
	s32 res;
	struct pwrctrl_priv *pwrctrl;
	u8 pslv;

_func_enter_;

	res = _SUCCESS;
	pwrctrl = adapter_to_pwrctl(padapter);
	pslv = PS_STATE_S2;

	down(&pwrctrl->lock);

	register_task_alive(pwrctrl, task);

	if (pwrctrl->bFwCurrentInPSMode == true)
	{
		RT_TRACE(_module_rtl871x_pwrctrl_c_, _drv_notice_,
				 ("%s: task=0x%x cpwm=0x%02x alives=0x%08x\n",
				  __FUNCTION__, task, pwrctrl->cpwm, pwrctrl->alives));

		if (pwrctrl->cpwm < pslv)
		{
			if (pwrctrl->cpwm < PS_STATE_S2)
				res = _FAIL;
			if (pwrctrl->rpwm < pslv)
				rtw_set_rpwm(padapter, pslv);
		}
	}

	up(&pwrctrl->lock);

	if (_FAIL == res)
	{
		if (pwrctrl->cpwm >= PS_STATE_S2)
			res = _SUCCESS;
	}

_func_exit_;

	return res;	
}

/*
 * Description:
 *	If task is done, call this func. to power down firmware again.
 *
 *	Constraint:
 *		1. this function will request pwrctrl->lock
 *
 * Return Value:
 *	none
 */
void rtw_unregister_task_alive(PADAPTER padapter, u32 task)
{
	struct pwrctrl_priv *pwrctrl;
	u8 pslv;

_func_enter_;

	pwrctrl = adapter_to_pwrctl(padapter);
	pslv = PS_STATE_S0;

	if ((rtw_btcoex_IsBtDisabled(padapter) == false)
		&& (rtw_btcoex_IsBtControlLps(padapter) == true))
	{
		u8 val8;

		val8 = rtw_btcoex_LpsVal(padapter);
		if (val8 & BIT(4))
			pslv = PS_STATE_S2;
	}

	down(&pwrctrl->lock);

	unregister_task_alive(pwrctrl, task);

	if ((pwrctrl->pwr_mode != PS_MODE_ACTIVE)
		&& (pwrctrl->bFwCurrentInPSMode == true))
	{
		RT_TRACE(_module_rtl871x_pwrctrl_c_, _drv_notice_,
				 ("%s: cpwm=0x%02x alives=0x%08x\n",
				  __FUNCTION__, pwrctrl->cpwm, pwrctrl->alives));

		if (pwrctrl->cpwm > pslv)
		{
			if ((pslv >= PS_STATE_S2) || (pwrctrl->alives == 0))
				rtw_set_rpwm(padapter, pslv);
		}
	}

	up(&pwrctrl->lock);

_func_exit_;
}

/*
 * Caller: rtw_xmit_thread
 * 
 * Check if the fw_pwrstate is okay for xmit.
 * If not (cpwm is less than S3), then the sub-routine
 * will raise the cpwm to be greater than or equal to S3. 
 *
 * Calling Context: Passive
 * 
 * Return Value:
 *	 _SUCCESS	rtw_xmit_thread can write fifo/txcmd afterwards.
 *	 _FAIL		rtw_xmit_thread can not do anything.
 */
s32 rtw_register_tx_alive(PADAPTER padapter)
{
	s32 res;
	struct pwrctrl_priv *pwrctrl;
	u8 pslv;

_func_enter_;

	res = _SUCCESS;
	pwrctrl = adapter_to_pwrctl(padapter);
	pslv = PS_STATE_S2;

	down(&pwrctrl->lock);

	register_task_alive(pwrctrl, XMIT_ALIVE);

	if (pwrctrl->bFwCurrentInPSMode == true)
	{
		RT_TRACE(_module_rtl871x_pwrctrl_c_, _drv_notice_,
				 ("rtw_register_tx_alive: cpwm=0x%02x alives=0x%08x\n",
				  pwrctrl->cpwm, pwrctrl->alives));

		if (pwrctrl->cpwm < pslv)
		{
			if (pwrctrl->cpwm < PS_STATE_S2)
				res = _FAIL;
			if (pwrctrl->rpwm < pslv)
				rtw_set_rpwm(padapter, pslv);
		}
	}

	up(&pwrctrl->lock);

	if (_FAIL == res)
	{
		if (pwrctrl->cpwm >= PS_STATE_S2)
			res = _SUCCESS;
	}

_func_exit_;

	return res;	
}

/*
 * Caller: rtw_cmd_thread
 *
 * Check if the fw_pwrstate is okay for issuing cmd.
 * If not (cpwm should be is less than S2), then the sub-routine
 * will raise the cpwm to be greater than or equal to S2.
 *
 * Calling Context: Passive
 *
 * Return Value:
 *	_SUCCESS	rtw_cmd_thread can issue cmds to firmware afterwards.
 *	_FAIL		rtw_cmd_thread can not do anything.
 */
s32 rtw_register_cmd_alive(PADAPTER padapter)
{
	s32 res;
	struct pwrctrl_priv *pwrctrl;
	u8 pslv;

_func_enter_;

	res = _SUCCESS;
	pwrctrl = adapter_to_pwrctl(padapter);
	pslv = PS_STATE_S2;

	down(&pwrctrl->lock);

	register_task_alive(pwrctrl, CMD_ALIVE);

	if (pwrctrl->bFwCurrentInPSMode == true)
	{
		RT_TRACE(_module_rtl871x_pwrctrl_c_, _drv_info_,
				 ("rtw_register_cmd_alive: cpwm=0x%02x alives=0x%08x\n",
				  pwrctrl->cpwm, pwrctrl->alives));

		if (pwrctrl->cpwm < pslv)
		{
			if (pwrctrl->cpwm < PS_STATE_S2)
				res = _FAIL;
			if (pwrctrl->rpwm < pslv)
				rtw_set_rpwm(padapter, pslv);
		}
	}

	up(&pwrctrl->lock);

	if (_FAIL == res)
	{
		if (pwrctrl->cpwm >= PS_STATE_S2)
			res = _SUCCESS;
	}

_func_exit_;

	return res;
}

/*
 * Caller: rx_isr
 *
 * Calling Context: Dispatch/ISR
 *
 * Return Value:
 *	_SUCCESS
 *	_FAIL
 */
s32 rtw_register_rx_alive(PADAPTER padapter)
{
	struct pwrctrl_priv *pwrctrl;

_func_enter_;

	pwrctrl = adapter_to_pwrctl(padapter);

	down(&pwrctrl->lock);

	register_task_alive(pwrctrl, RECV_ALIVE);
	RT_TRACE(_module_rtl871x_pwrctrl_c_, _drv_notice_,
			 ("rtw_register_rx_alive: cpwm=0x%02x alives=0x%08x\n",
			  pwrctrl->cpwm, pwrctrl->alives));

	up(&pwrctrl->lock);

_func_exit_;

	return _SUCCESS;
}

/*
 * Caller: evt_isr or evt_thread
 *
 * Calling Context: Dispatch/ISR or Passive
 *
 * Return Value:
 *	_SUCCESS
 *	_FAIL
 */
s32 rtw_register_evt_alive(PADAPTER padapter)
{
	struct pwrctrl_priv *pwrctrl;

_func_enter_;

	pwrctrl = adapter_to_pwrctl(padapter);

	down(&pwrctrl->lock);

	register_task_alive(pwrctrl, EVT_ALIVE);
	RT_TRACE(_module_rtl871x_pwrctrl_c_, _drv_notice_,
			 ("rtw_register_evt_alive: cpwm=0x%02x alives=0x%08x\n",
			  pwrctrl->cpwm, pwrctrl->alives));

	up(&pwrctrl->lock);

_func_exit_;

	return _SUCCESS;
}

/*
 * Caller: ISR
 *
 * If ISR's txdone,
 * No more pkts for TX,
 * Then driver shall call this fun. to power down firmware again.
 */
void rtw_unregister_tx_alive(PADAPTER padapter)
{
	struct pwrctrl_priv *pwrctrl;
	u8 pslv;

_func_enter_;

	pwrctrl = adapter_to_pwrctl(padapter);
	pslv = PS_STATE_S0;

	if ((rtw_btcoex_IsBtDisabled(padapter) == false)
		&& (rtw_btcoex_IsBtControlLps(padapter) == true))
	{
		u8 val8;

		val8 = rtw_btcoex_LpsVal(padapter);
		if (val8 & BIT(4))
			pslv = PS_STATE_S2;
	}

	down(&pwrctrl->lock);

	unregister_task_alive(pwrctrl, XMIT_ALIVE);

	if ((pwrctrl->pwr_mode != PS_MODE_ACTIVE)
		&& (pwrctrl->bFwCurrentInPSMode == true))
	{
		RT_TRACE(_module_rtl871x_pwrctrl_c_, _drv_notice_,
				 ("%s: cpwm=0x%02x alives=0x%08x\n",
				  __FUNCTION__, pwrctrl->cpwm, pwrctrl->alives));

		if (pwrctrl->cpwm > pslv)
		{
			if ((pslv >= PS_STATE_S2) || (pwrctrl->alives == 0))
				rtw_set_rpwm(padapter, pslv);
		}
	}

	up(&pwrctrl->lock);

_func_exit_;
}

/*
 * Caller: ISR
 *
 * If all commands have been done,
 * and no more command to do,
 * then driver shall call this fun. to power down firmware again.
 */
void rtw_unregister_cmd_alive(PADAPTER padapter)
{
	struct pwrctrl_priv *pwrctrl;
	u8 pslv;

_func_enter_;

	pwrctrl = adapter_to_pwrctl(padapter);
	pslv = PS_STATE_S0;

	if ((rtw_btcoex_IsBtDisabled(padapter) == false)
		&& (rtw_btcoex_IsBtControlLps(padapter) == true))
	{
		u8 val8;

		val8 = rtw_btcoex_LpsVal(padapter);
		if (val8 & BIT(4))
			pslv = PS_STATE_S2;
	}

	down(&pwrctrl->lock);

	unregister_task_alive(pwrctrl, CMD_ALIVE);

	if ((pwrctrl->pwr_mode != PS_MODE_ACTIVE)
		&& (pwrctrl->bFwCurrentInPSMode == true))
	{
		RT_TRACE(_module_rtl871x_pwrctrl_c_, _drv_info_,
				 ("%s: cpwm=0x%02x alives=0x%08x\n",
				  __FUNCTION__, pwrctrl->cpwm, pwrctrl->alives));

		if (pwrctrl->cpwm > pslv)
		{
			if ((pslv >= PS_STATE_S2) || (pwrctrl->alives == 0))
				rtw_set_rpwm(padapter, pslv);
		}
	}

	up(&pwrctrl->lock);

_func_exit_;
}

/*
 * Caller: ISR
 */
void rtw_unregister_rx_alive(PADAPTER padapter)
{
	struct pwrctrl_priv *pwrctrl;

_func_enter_;

	pwrctrl = adapter_to_pwrctl(padapter);

	down(&pwrctrl->lock);

	unregister_task_alive(pwrctrl, RECV_ALIVE);

	RT_TRACE(_module_rtl871x_pwrctrl_c_, _drv_notice_,
			 ("rtw_unregister_rx_alive: cpwm=0x%02x alives=0x%08x\n",
			  pwrctrl->cpwm, pwrctrl->alives));

	up(&pwrctrl->lock);

_func_exit_;
}

void rtw_unregister_evt_alive(PADAPTER padapter)
{
	struct pwrctrl_priv *pwrctrl;

_func_enter_;

	pwrctrl = adapter_to_pwrctl(padapter);

	unregister_task_alive(pwrctrl, EVT_ALIVE);

	RT_TRACE(_module_rtl871x_pwrctrl_c_, _drv_notice_,
			 ("rtw_unregister_evt_alive: cpwm=0x%02x alives=0x%08x\n",
			  pwrctrl->cpwm, pwrctrl->alives));

	up(&pwrctrl->lock);

_func_exit_;
}

void rtw_init_pwrctrl_priv(PADAPTER padapter)
{
	struct pwrctrl_priv *pwrctrlpriv = adapter_to_pwrctl(padapter);

_func_enter_;

	sema_init(&pwrctrlpriv->lock, 1);
	sema_init(&pwrctrlpriv->check_32k_lock, 1);
	pwrctrlpriv->rf_pwrstate = rf_on;
	pwrctrlpriv->ips_enter_cnts=0;
	pwrctrlpriv->ips_leave_cnts=0;
	pwrctrlpriv->bips_processing = false;

	pwrctrlpriv->ips_mode = padapter->registrypriv.ips_mode;
	pwrctrlpriv->ips_mode_req = padapter->registrypriv.ips_mode;

	pwrctrlpriv->pwr_state_check_interval = RTW_PWR_STATE_CHK_INTERVAL;
	pwrctrlpriv->pwr_state_check_cnts = 0;
	pwrctrlpriv->bInternalAutoSuspend = false;
	pwrctrlpriv->bInSuspend = false;
	pwrctrlpriv->bkeepfwalive = false;

	pwrctrlpriv->LpsIdleCount = 0;
	//pwrctrlpriv->FWCtrlPSMode =padapter->registrypriv.power_mgnt;// PS_MODE_MIN;
	if (padapter->registrypriv.mp_mode == 1)
		pwrctrlpriv->power_mgnt =PS_MODE_ACTIVE ;
	else	
		pwrctrlpriv->power_mgnt =padapter->registrypriv.power_mgnt;// PS_MODE_MIN;
	pwrctrlpriv->bLeisurePs = (PS_MODE_ACTIVE != pwrctrlpriv->power_mgnt)?true:false;

	pwrctrlpriv->bFwCurrentInPSMode = false;

	pwrctrlpriv->rpwm = 0;
	pwrctrlpriv->cpwm = PS_STATE_S4;

	pwrctrlpriv->pwr_mode = PS_MODE_ACTIVE;
	pwrctrlpriv->smart_ps = padapter->registrypriv.smart_ps;
	pwrctrlpriv->bcn_ant_mode = 0;
	pwrctrlpriv->dtim = 0;

	pwrctrlpriv->tog = 0x80;

	rtw_hal_set_hwreg(padapter, HW_VAR_SET_RPWM, (u8 *)(&pwrctrlpriv->rpwm));

	_init_workitem(&pwrctrlpriv->cpwm_event, cpwm_event_callback, NULL);

	pwrctrlpriv->brpwmtimeout = false;
	_init_workitem(&pwrctrlpriv->rpwmtimeoutwi, rpwmtimeout_workitem_callback, NULL);
	_init_timer(&pwrctrlpriv->pwr_rpwm_timer, padapter->pnetdev, pwr_rpwm_timeout_handler, padapter);

	rtw_init_timer(&pwrctrlpriv->pwr_state_check_timer, padapter, pwr_state_check_handler);

	pwrctrlpriv->wowlan_mode = false;
	pwrctrlpriv->wowlan_ap_mode = false;

#ifdef CONFIG_PNO_SUPPORT
	pwrctrlpriv->pno_inited = false;
	pwrctrlpriv->pnlo_info = NULL;
	pwrctrlpriv->pscan_info = NULL;
	pwrctrlpriv->pno_ssid_list = NULL;
	pwrctrlpriv->pno_in_resume = true;
#endif

_func_exit_;

}


void rtw_free_pwrctrl_priv(PADAPTER adapter)
{
	struct pwrctrl_priv *pwrctrlpriv = adapter_to_pwrctl(adapter);

_func_enter_;

	//memset((unsigned char *)pwrctrlpriv, 0, sizeof(struct pwrctrl_priv));

#ifdef CONFIG_PNO_SUPPORT
	if (pwrctrlpriv->pnlo_info != NULL)
		printk("****** pnlo_info memory leak********\n");

	if (pwrctrlpriv->pscan_info != NULL)
		printk("****** pscan_info memory leak********\n");

	if (pwrctrlpriv->pno_ssid_list != NULL)
		printk("****** pno_ssid_list memory leak********\n");
#endif

_func_exit_;
}

u8 rtw_interface_ps_func(_adapter *padapter,HAL_INTF_PS_FUNC efunc_id,u8* val)
{
	u8 bResult = true;
	rtw_hal_intf_ps_func(padapter,efunc_id,val);
	
	return bResult;
}


inline void rtw_set_ips_deny(_adapter *padapter, u32 ms)
{
	struct pwrctrl_priv *pwrpriv = adapter_to_pwrctl(padapter);
	pwrpriv->ips_deny_time = jiffies + msecs_to_jiffies(ms);
}

/*
* rtw_pwr_wakeup - Wake the NIC up from: 1)IPS. 2)USB autosuspend
* @adapter: pointer to _adapter structure
* @ips_deffer_ms: the ms wiil prevent from falling into IPS after wakeup
* Return _SUCCESS or _FAIL
*/

int _rtw_pwr_wakeup(_adapter *padapter, u32 ips_deffer_ms, const char *caller)
{
	struct dvobj_priv *dvobj = adapter_to_dvobj(padapter);
	struct pwrctrl_priv *pwrpriv = dvobj_to_pwrctl(dvobj);
	struct mlme_priv *pmlmepriv;
	int ret = _SUCCESS;
	int i;
	unsigned long start = jiffies;
	unsigned long deny_time = jiffies + msecs_to_jiffies(ips_deffer_ms);

	/* for LPS */
	LeaveAllPowerSaveMode(padapter);

	/* IPS still bound with primary adapter */
	padapter = GET_PRIMARY_ADAPTER(padapter);
	pmlmepriv = &padapter->mlmepriv;

	if (time_before(pwrpriv->ips_deny_time, deny_time))
		pwrpriv->ips_deny_time = deny_time;


	if (pwrpriv->ps_processing) {
		DBG_871X("%s wait ps_processing...\n", __func__);
		while (pwrpriv->ps_processing && jiffies_to_msecs(jiffies - start) <= 3000)
			msleep(10);
		if (pwrpriv->ps_processing)
			DBG_871X("%s wait ps_processing timeout\n", __func__);
		else
			DBG_871X("%s wait ps_processing done\n", __func__);
	}

	if (pwrpriv->bInternalAutoSuspend == false && pwrpriv->bInSuspend) {
		DBG_871X("%s wait bInSuspend...\n", __func__);
		while (pwrpriv->bInSuspend 
			&& jiffies_to_msecs(jiffies - start) <= 3000
		) {
			msleep(10);
		}
		if (pwrpriv->bInSuspend)
			DBG_871X("%s wait bInSuspend timeout\n", __func__);
		else
			DBG_871X("%s wait bInSuspend done\n", __func__);
	}

	//System suspend is not allowed to wakeup
	if((pwrpriv->bInternalAutoSuspend == false) && (true == pwrpriv->bInSuspend )){
		ret = _FAIL;
		goto exit;
	}

	//block???
	if((pwrpriv->bInternalAutoSuspend == true)  && (padapter->net_closed == true)) {
		ret = _FAIL;
		goto exit;
	}

	//I think this should be check in IPS, LPS, autosuspend functions...
	if (check_fwstate(pmlmepriv, _FW_LINKED) == true)
	{
		ret = _SUCCESS;
		goto exit;
	}	

	if(rf_off == pwrpriv->rf_pwrstate )
	{		
		{
			DBG_8192C("%s call ips_leave....\n",__FUNCTION__);
			if(_FAIL ==  ips_leave(padapter))
			{
				DBG_8192C("======> ips_leave fail.............\n");
				ret = _FAIL;
				goto exit;
			}
		}
	}

	//TODO: the following checking need to be merged...
	if(padapter->bDriverStopped
		|| !padapter->bup
		|| !padapter->hw_init_completed
	){
		DBG_8192C("%s: bDriverStopped=%d, bup=%d, hw_init_completed=%u\n"
			, caller
		   	, padapter->bDriverStopped
		   	, padapter->bup
		   	, padapter->hw_init_completed);
		ret= false;
		goto exit;
	}

exit:
	deny_time = jiffies + msecs_to_jiffies(ips_deffer_ms);
	if (time_before(pwrpriv->ips_deny_time, deny_time))
		pwrpriv->ips_deny_time = deny_time;
	return ret;

}

int rtw_pm_set_lps(_adapter *padapter, u8 mode)
{
	int	ret = 0;	
	struct pwrctrl_priv *pwrctrlpriv = adapter_to_pwrctl(padapter);
	
	if ( mode < PS_MODE_NUM )
	{
		if(pwrctrlpriv->power_mgnt !=mode)
		{
			if(PS_MODE_ACTIVE == mode)
			{
				LeaveAllPowerSaveMode(padapter);
			}
			else
			{
				pwrctrlpriv->LpsIdleCount = 2;
			}
			pwrctrlpriv->power_mgnt = mode;
			pwrctrlpriv->bLeisurePs = (PS_MODE_ACTIVE != pwrctrlpriv->power_mgnt)?true:false;
		}
	}
	else
	{
		ret = -EINVAL;
	}

	return ret;
}

int rtw_pm_set_ips(_adapter *padapter, u8 mode)
{
	struct pwrctrl_priv *pwrctrlpriv = adapter_to_pwrctl(padapter);

	if( mode == IPS_NORMAL || mode == IPS_LEVEL_2 ) {
		rtw_ips_mode_req(pwrctrlpriv, mode);
		DBG_871X("%s %s\n", __FUNCTION__, mode == IPS_NORMAL?"IPS_NORMAL":"IPS_LEVEL_2");
		return 0;
	} 
	else if(mode ==IPS_NONE){
		rtw_ips_mode_req(pwrctrlpriv, mode);
		DBG_871X("%s %s\n", __FUNCTION__, "IPS_NONE");
		if((padapter->bSurpriseRemoved ==0)&&(_FAIL == rtw_pwr_wakeup(padapter)) )
			return -EFAULT;
	}
	else {
		return -EINVAL;
	}
	return 0;
}

/*
 * ATTENTION:
 *	This function will request pwrctrl LOCK!
 */
void rtw_ps_deny(PADAPTER padapter, PS_DENY_REASON reason)
{
	struct pwrctrl_priv *pwrpriv;
	s32 ret;


//	DBG_871X("+" FUNC_ADPT_FMT ": Request PS deny for %d (0x%08X)\n",
//		FUNC_ADPT_ARG(padapter), reason, BIT(reason));

	pwrpriv = adapter_to_pwrctl(padapter);

	down(&pwrpriv->lock);
	if (pwrpriv->ps_deny & BIT(reason))
	{
		DBG_871X(FUNC_ADPT_FMT ": [WARNING] Reason %d had been set before!!\n",
			FUNC_ADPT_ARG(padapter), reason);
	}
	pwrpriv->ps_deny |= BIT(reason);
	up(&pwrpriv->lock);

//	DBG_871X("-" FUNC_ADPT_FMT ": Now PS deny for 0x%08X\n",
//		FUNC_ADPT_ARG(padapter), pwrpriv->ps_deny);
}

/*
 * ATTENTION:
 *	This function will request pwrctrl LOCK!
 */
void rtw_ps_deny_cancel(PADAPTER padapter, PS_DENY_REASON reason)
{
	struct pwrctrl_priv *pwrpriv;


//	DBG_871X("+" FUNC_ADPT_FMT ": Cancel PS deny for %d(0x%08X)\n",
//		FUNC_ADPT_ARG(padapter), reason, BIT(reason));

	pwrpriv = adapter_to_pwrctl(padapter);

	down(&pwrpriv->lock);
	if ((pwrpriv->ps_deny & BIT(reason)) == 0)
	{
		DBG_871X(FUNC_ADPT_FMT ": [ERROR] Reason %d had been canceled before!!\n",
			FUNC_ADPT_ARG(padapter), reason);
	}
	pwrpriv->ps_deny &= ~BIT(reason);
	up(&pwrpriv->lock);

//	DBG_871X("-" FUNC_ADPT_FMT ": Now PS deny for 0x%08X\n",
//		FUNC_ADPT_ARG(padapter), pwrpriv->ps_deny);
}

/*
 * ATTENTION:
 *	Before calling this function pwrctrl lock should be occupied already,
 *	otherwise it may return incorrect value.
 */
u32 rtw_ps_deny_get(PADAPTER padapter)
{
	u32 deny;


	deny = adapter_to_pwrctl(padapter)->ps_deny;

	return deny;
}

