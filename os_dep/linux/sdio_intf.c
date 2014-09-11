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
#define _HCI_INTF_C_

#include <drv_conf.h>
#include <osdep_service.h>
#include <drv_types.h>
#include <recv_osdep.h>
#include <xmit_osdep.h>
#include <rtw_version.h>

#ifndef CONFIG_SDIO_HCI
#error "CONFIG_SDIO_HCI shall be on!\n"
#endif

#include <linux/mmc/sdio_func.h>
#include <linux/mmc/sdio_ids.h>

#if defined(CONFIG_WOWLAN) || defined(CONFIG_PLATFORM_SPRD)
#include <linux/mmc/host.h>
#include <linux/mmc/card.h>
#endif

#ifdef CONFIG_PLATFORM_HUAWEI_K3V2
#include <custom_gpio.h>
#endif //CONFIG_PLATFORM_HUAWEI_K3V2

#ifdef CONFIG_PLATFORM_SPRD
#include <custom_gpio.h>
#endif // CONFIG_PLATFORM_SPRD

#ifdef PLATFORM_LEADCORE_1813
#include <custom_gpio.h>
#endif //


#include <hal_intf.h>
#include <sdio_hal.h>
#include <sdio_ops.h>

#include <rtw_android.h>

#ifdef CONFIG_GPIO_WAKEUP
#include <rtw_io.h>

unsigned int oob_irq = 0;

static irqreturn_t gpio_hostwakeup_irq_thread(int irq, void *data)
{
	PADAPTER padapter = (PADAPTER)data;

	DBG_871X_LEVEL(_drv_always_, "gpio_hostwakeup_irq_thread\n");
	/* Disable interrupt before calling handler */
#ifndef CONFIG_PLATFORM_HUAWEI_K3V2
	disable_irq_nosync(oob_irq);
#endif

	rtw_lock_suspend_timeout(HZ/2);

	return IRQ_HANDLED;
}

static u8 gpio_hostwakeup_alloc_irq(PADAPTER padapter)
{
	int err;

	if (oob_irq == 0)
		return _FAIL;

	/* dont set it IRQF_TRIGGER_LOW, or wowlan */
	/* power is high after suspend */
	/* and failing can prevent can not sleep issue if */
	/* wifi gpio12 pin is not linked with CPU */
	err = request_irq(oob_irq, gpio_hostwakeup_irq_thread,
#ifdef CONFIG_PLATFORM_HUAWEI_K3V2
			IRQF_NO_SUSPEND | IRQF_TRIGGER_FALLING,
#else
			IRQF_TRIGGER_LOW | IRQF_ONESHOT,
#endif
			"rtw_wifi_gpio_wakeup", padapter);
	//err = request_threaded_irq(oob_irq, gpio_hostwakeup_irq_thread, NULL,
			//IRQF_TRIGGER_FALLING,
			//"rtw_wifi_gpio_wakeup", padapter);
	if (err < 0) {
		DBG_871X("Oops: can't allocate gpio irq %d err:%d\n", oob_irq, err);
		return _FAIL;
	} else {
		DBG_871X("allocate gpio irq %d ok\n", oob_irq);
	}

	enable_irq_wake(oob_irq);

	return _SUCCESS;
}

static void gpio_hostwakeup_free_irq(PADAPTER padapter)
{
	if (oob_irq == 0)
		return;

	disable_irq_wake(oob_irq);
	free_irq(oob_irq, padapter);
}
#endif

#ifdef CONFIG_PLATFORM_ARM_SUN4I
#if defined(CONFIG_MMC_SUNXI_POWER_CONTROL)
#ifdef CONFIG_RTL8723B
#define SDIOID 3
#endif
#ifdef CONFIG_RTL8188E
#define SDIOID (CONFIG_CHIP_ID==1123 ? 3 : 1)
#endif
#define SUNXI_SDIO_WIFI_NUM_RTL8189ES  10
#define SUNXI_SDIO_WIFI_NUM_RTL8723BS  11

extern void sunximmc_rescan_card(unsigned id, unsigned insert);
extern int mmc_pm_get_mod_type(void);
extern int mmc_pm_gpio_ctrl(char* name, int level);
/*
*	rtl8189es_shdn	= port:PH09<1><default><default><0>
*	rtl8189es_wakeup	= port:PH10<1><default><default><1>
*	rtl8189es_vdd_en  = port:PH11<1><default><default><0>
*	rtl8189es_vcc_en  = port:PH12<1><default><default><0>
*/
int rtl8723bs_sdio_powerup(void)
{
    mmc_pm_gpio_ctrl("rtk_rtl8723bs_wl_dis", 1);
    return 0;
}
int rtl8723bs_sdio_poweroff(void)
{
    mmc_pm_gpio_ctrl("rtk_rtl8723bs_wl_dis", 0);
    return 0;
}

int rtl8189es_sdio_powerup(void)
{
	mmc_pm_gpio_ctrl("rtl8189es_vdd_en", 1);
	udelay(100);
	mmc_pm_gpio_ctrl("rtl8189es_vcc_en", 1);
	udelay(50);
	mmc_pm_gpio_ctrl("rtl8189es_shdn", 1);
	return 0;
}
int rtl8189es_sdio_poweroff(void)
{
	mmc_pm_gpio_ctrl("rtl8189es_shdn", 0);
	mmc_pm_gpio_ctrl("rtl8189es_vcc_en", 0);
	mmc_pm_gpio_ctrl("rtl8189es_vdd_en", 0);
	return 0;
}
#endif //defined(CONFIG_MMC_SUNXI_POWER_CONTROL)
#endif //CONFIG_PLATFORM_ARM_SUN4I

#ifndef dev_to_sdio_func
#define dev_to_sdio_func(d)     container_of(d, struct sdio_func, dev)
#endif

#ifdef CONFIG_WOWLAN
static struct mmc_host *mmc_host = NULL;
#endif

static const struct sdio_device_id sdio_ids[] = {
#ifdef CONFIG_RTL8723A
	{ SDIO_DEVICE(0x024c, 0x8723) },
#endif //CONFIG_RTL8723A
#ifdef CONFIG_RTL8723B
	{ SDIO_DEVICE(0x024c, 0xB723) },
#endif
#ifdef CONFIG_RTL8188E
	{ SDIO_DEVICE(0x024c, 0x8179) },
#endif //CONFIG_RTL8188E

#if defined(RTW_ENABLE_WIFI_CONTROL_FUNC) /* temporarily add this to accept all sdio wlan id */
	{ SDIO_DEVICE_CLASS(SDIO_CLASS_WLAN) },
#endif
//	{ /* end: all zeroes */				},
};

static int rtw_drv_init(struct sdio_func *func, const struct sdio_device_id *id);
static void rtw_dev_remove(struct sdio_func *func);
static int rtw_sdio_resume(struct device *dev);
static int rtw_sdio_suspend(struct device *dev);

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,29))
static const struct dev_pm_ops rtw_sdio_pm_ops = {
	.suspend	= rtw_sdio_suspend,
	.resume	= rtw_sdio_resume,
};
#endif

struct sdio_drv_priv {
	struct sdio_driver r871xs_drv;
	int drv_registered;
};

static struct sdio_drv_priv sdio_drvpriv = {
	.r871xs_drv.probe = rtw_drv_init,
	.r871xs_drv.remove = rtw_dev_remove,
	.r871xs_drv.name = (char*)DRV_NAME,
	.r871xs_drv.id_table = sdio_ids,
	#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,29))
	.r871xs_drv.drv = {
		.pm = &rtw_sdio_pm_ops,
	}
	#endif
};

static void sd_sync_int_hdl(struct sdio_func *func)
{
	struct dvobj_priv *psdpriv;


	psdpriv = sdio_get_drvdata(func);

	if (!psdpriv->if1) {
		DBG_871X("%s if1 == NULL\n", __func__);
		return;
	}

	sd_int_hdl(psdpriv->if1);
}

int sdio_alloc_irq(struct dvobj_priv *dvobj)
{
	PSDIO_DATA psdio_data;
	struct sdio_func *func;
	int err;

	psdio_data = &dvobj->intf_data;
	func = psdio_data->func;

	sdio_claim_host(func);

	err = sdio_claim_irq(func, &sd_sync_int_hdl);
	if (err)
		DBG_871X_LEVEL(_drv_err_,"%s: sdio_claim_irq FAIL(%d)!\n", __func__, err);
	else
		dvobj->irq_alloc = 1;

	sdio_release_host(func);

	return err?_FAIL:_SUCCESS;
}

void sdio_free_irq(struct dvobj_priv *dvobj)
{
	PSDIO_DATA psdio_data;
	struct sdio_func *func;
	int err;

	if (dvobj->irq_alloc) {
		psdio_data = &dvobj->intf_data;
		func = psdio_data->func;

		if (func) {
			sdio_claim_host(func);
			err = sdio_release_irq(func);
			if (err)
				DBG_871X_LEVEL(_drv_err_,"%s: sdio_release_irq FAIL(%d)!\n", __func__, err);
			sdio_release_host(func);
		}
		dvobj->irq_alloc = 0;
	}
}

static u32 sdio_init(struct dvobj_priv *dvobj)
{
	PSDIO_DATA psdio_data;
	struct sdio_func *func;
	int err;

_func_enter_;

	psdio_data = &dvobj->intf_data;
	func = psdio_data->func;

	//3 1. init SDIO bus
	sdio_claim_host(func);

	err = sdio_enable_func(func);
	if (err) {
		DBG_871X_LEVEL(_drv_err_, "%s: sdio_enable_func FAIL(%d)!\n", __func__, err);
		goto release;
	}

	err = sdio_set_block_size(func, 512);
	if (err) {
		DBG_871X_LEVEL(_drv_err_, "%s: sdio_set_block_size FAIL(%d)!\n", __func__, err);
		goto release;
	}
	psdio_data->block_transfer_len = 512;
	psdio_data->tx_block_mode = 1;
	psdio_data->rx_block_mode = 1;

release:
	sdio_release_host(func);

exit:
_func_exit_;

	if (err) return _FAIL;
	return _SUCCESS;
}

static void sdio_deinit(struct dvobj_priv *dvobj)
{
	struct sdio_func *func;
	int err;


	RT_TRACE(_module_hci_intfs_c_, _drv_notice_, ("+sdio_deinit\n"));

	func = dvobj->intf_data.func;

	if (func) {
		sdio_claim_host(func);
		err = sdio_disable_func(func);
		if (err)
			DBG_8192C(KERN_ERR "%s: sdio_disable_func(%d)\n", __func__, err);

		if (dvobj->irq_alloc) {
			err = sdio_release_irq(func);
			if (err)
				DBG_8192C(KERN_ERR "%s: sdio_release_irq(%d)\n", __func__, err);
		}

		sdio_release_host(func);
	}
}
static struct dvobj_priv *sdio_dvobj_init(struct sdio_func *func)
{
	int status = _FAIL;
	struct dvobj_priv *dvobj = NULL;
	PSDIO_DATA psdio;
_func_enter_;

	if ((dvobj = (struct dvobj_priv*)rtw_zmalloc(sizeof(*dvobj))) == NULL) {
		goto exit;
	}

	_rtw_mutex_init(&dvobj->hw_init_mutex);
	_rtw_mutex_init(&dvobj->h2c_fwcmd_mutex);
	_rtw_mutex_init(&dvobj->setch_mutex);
	_rtw_mutex_init(&dvobj->setbw_mutex);

	sdio_set_drvdata(func, dvobj);

	psdio = &dvobj->intf_data;
	psdio->func = func;

	if (sdio_init(dvobj) != _SUCCESS) {
		RT_TRACE(_module_hci_intfs_c_, _drv_err_, ("%s: initialize SDIO Failed!\n", __FUNCTION__));
		goto free_dvobj;
	}

	status = _SUCCESS;

free_dvobj:
	if (status != _SUCCESS && dvobj) {
		sdio_set_drvdata(func, NULL);
		_rtw_mutex_free(&dvobj->hw_init_mutex);
		_rtw_mutex_free(&dvobj->h2c_fwcmd_mutex);
		_rtw_mutex_free(&dvobj->setch_mutex);
		_rtw_mutex_free(&dvobj->setbw_mutex);
		rtw_mfree((u8*)dvobj, sizeof(*dvobj));
		dvobj = NULL;
	}
exit:
_func_exit_;
	return dvobj;
}

static void sdio_dvobj_deinit(struct sdio_func *func)
{
	struct dvobj_priv *dvobj = sdio_get_drvdata(func);
_func_enter_;

	sdio_set_drvdata(func, NULL);
	if (dvobj) {
		sdio_deinit(dvobj);
		_rtw_mutex_free(&dvobj->hw_init_mutex);
		_rtw_mutex_free(&dvobj->h2c_fwcmd_mutex);
		_rtw_mutex_free(&dvobj->setch_mutex);
		_rtw_mutex_free(&dvobj->setbw_mutex);
		rtw_mfree((u8*)dvobj, sizeof(*dvobj));
	}

_func_exit_;
	return;
}
static void decide_chip_type_by_device_id(PADAPTER padapter, u32 id)
{
	padapter->chip_type = NULL_CHIP_TYPE;

#if 0
	switch (id)
	{
		case 0x8723:
			padapter->chip_type = RTL8723A;
			padapter->HardwareType = HARDWARE_TYPE_RTL8723AS;
			break;
		case 0x8179:
			padapter->chip_type = RTL8188E;
			padapter->HardwareType = HARDWARE_TYPE_RTL8188ES;
			break;
	}
#else
#if defined(CONFIG_RTL8723A)
	padapter->chip_type = RTL8723A;
	padapter->HardwareType = HARDWARE_TYPE_RTL8723AS;
#elif defined(CONFIG_RTL8188E)
	padapter->chip_type = RTL8188E;
	padapter->HardwareType = HARDWARE_TYPE_RTL8188ES;
#elif defined(CONFIG_RTL8723B)
	padapter->chip_type = RTL8723B;
	padapter->HardwareType = HARDWARE_TYPE_RTL8723BS;
#endif
#endif
}

static void sd_intf_start(PADAPTER padapter)
{
	if (padapter == NULL) {
		DBG_8192C(KERN_ERR "%s: padapter is NULL!\n", __func__);
		return;
	}

	// hal dep
	rtw_hal_enable_interrupt(padapter);
}

static void sd_intf_stop(PADAPTER padapter)
{
	if (padapter == NULL) {
		DBG_8192C(KERN_ERR "%s: padapter is NULL!\n", __func__);
		return;
	}

	// hal dep
	rtw_hal_disable_interrupt(padapter);
}

/*
 * Do deinit job corresponding to netdev_open()
 */
static void rtw_dev_unload(PADAPTER padapter)
{
	struct net_device *pnetdev = (struct net_device*)padapter->pnetdev;
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;
	struct pwrctrl_priv *pwrctrlpriv = adapter_to_pwrctl(padapter);

	RT_TRACE(_module_hci_intfs_c_, _drv_notice_, ("+rtw_dev_unload\n"));


	if (padapter->bup == _TRUE)
	{
		// stop TX
//		val8 = 0xFF;
//		rtw_hal_set_hwreg(padapter, HW_VAR_TXPAUSE,&val8);

#if 0
		if (padapter->intf_stop)
			padapter->intf_stop(padapter);
#else
			sd_intf_stop(padapter);
#endif

		padapter->bDriverStopped = _TRUE;

		RT_TRACE(_module_hci_intfs_c_, _drv_notice_, ("@ rtw_dev_unload: stop intf complete!\n"));

		if (!pwrctrlpriv->bInternalAutoSuspend)
			rtw_stop_drv_threads(padapter);

		RT_TRACE(_module_hci_intfs_c_, _drv_notice_, ("@ rtw_dev_unload: stop thread complete!\n"));

		if (padapter->bSurpriseRemoved == _FALSE)
		{
#ifdef CONFIG_WOWLAN
			if (pwrctrlpriv->bSupportRemoteWakeup == _TRUE &&
				pwrctrlpriv->wowlan_mode ==_TRUE) {
				DBG_871X("%s bSupportWakeOnWlan==_TRUE  do not run rtw_hal_deinit()\n",__FUNCTION__);
			}
			else
#endif
			{
				//amy modify 20120221 for power seq is different between driver open and ips
				rtw_hal_deinit(padapter);
			}
			padapter->bSurpriseRemoved = _TRUE;
		}
		RT_TRACE(_module_hci_intfs_c_, _drv_notice_, ("@ rtw_dev_unload: deinit hal complelt!\n"));

		padapter->bup = _FALSE;
	}
	else {
		padapter->bDriverStopped = _TRUE;
		RT_TRACE(_module_hci_intfs_c_, _drv_notice_, ("rtw_dev_unload: bup==_FALSE\n"));
	}

	RT_TRACE(_module_hci_intfs_c_, _drv_notice_, ("-rtw_dev_unload\n"));
}

_adapter *rtw_sdio_if1_init(struct dvobj_priv *dvobj, const struct sdio_device_id  *pdid)
{
	int status = _FAIL;
	struct net_device *pnetdev;
	PADAPTER padapter = NULL;

	if ((padapter = (_adapter *)rtw_zvmalloc(sizeof(*padapter))) == NULL) {
		goto exit;
	}
	padapter->dvobj = dvobj;
	dvobj->if1 = padapter;

	dvobj->padapters[dvobj->iface_nums++] = padapter;
	padapter->iface_id = IFACE_ID0;

	padapter->bDriverStopped=_TRUE;
	padapter->drv_in_test = NO_TEST;

#if defined(CONFIG_CONCURRENT_MODE) || defined(CONFIG_DUALMAC_CONCURRENT)
	//set adapter_type/iface type for primary padapter
	padapter->isprimary = _TRUE;
	padapter->adapter_type = PRIMARY_ADAPTER;
	#ifndef CONFIG_HWPORT_SWAP
	padapter->iface_type = IFACE_PORT0;
	#else
	padapter->iface_type = IFACE_PORT1;
	#endif
#endif

	padapter->interface_type = RTW_SDIO;
	decide_chip_type_by_device_id(padapter, (u32)pdid->device);

	//3 1. init network device data
	pnetdev = rtw_init_netdev(padapter);
	if (!pnetdev)
		goto free_adapter;

	SET_NETDEV_DEV(pnetdev, dvobj_to_dev(dvobj));

	padapter = rtw_netdev_priv(pnetdev);
	padapter->bDriverStopped=_TRUE;

#ifdef CONFIG_IOCTL_CFG80211
	rtw_wdev_alloc(padapter, dvobj_to_dev(dvobj));
#endif

	//3 3. init driver special setting, interface, OS and hardware relative

	//4 3.1 set hardware operation functions
	hal_set_hal_ops(padapter);


	//3 5. initialize Chip version
	padapter->intf_start = &sd_intf_start;
	padapter->intf_stop = &sd_intf_stop;

	if (rtw_init_io_priv(padapter, sdio_set_intf_ops) == _FAIL)
	{
		RT_TRACE(_module_hci_intfs_c_, _drv_err_,
			("rtw_drv_init: Can't init io_priv\n"));
		goto free_hal_data;
	}

	rtw_hal_read_chip_version(padapter);

	rtw_hal_chip_configure(padapter);


	//3 6. read efuse/eeprom data
	rtw_hal_read_chip_info(padapter);

	//3 7. init driver common data
	if (rtw_init_drv_sw(padapter) == _FAIL) {
		RT_TRACE(_module_hci_intfs_c_, _drv_err_,
			 ("rtw_drv_init: Initialize driver software resource Failed!\n"));
		goto free_hal_data;
	}

	//3 8. get WLan MAC address
	// alloc dev name after read efuse.
	rtw_init_netdev_name(pnetdev, padapter->registrypriv.ifname);

	rtw_macaddr_cfg(padapter->eeprompriv.mac_addr);

	rtw_init_wifidirect_addrs(padapter, padapter->eeprompriv.mac_addr, padapter->eeprompriv.mac_addr);

	rtw_hal_disable_interrupt(padapter);

	DBG_871X("bDriverStopped:%d, bSurpriseRemoved:%d, bup:%d, hw_init_completed:%d\n"
		,padapter->bDriverStopped
		,padapter->bSurpriseRemoved
		,padapter->bup
		,padapter->hw_init_completed
	);

#ifdef CONFIG_HOSTAPD_MLME
	hostapd_mode_init(padapter);
#endif

#ifdef CONFIG_PLATFORM_RTD2880B
	DBG_871X("wlan link up\n");
	rtd2885_wlan_netlink_sendMsg("linkup", "8712");
#endif

#ifdef RTK_DMP_PLATFORM
	rtw_proc_init_one(pnetdev);
#endif

	status = _SUCCESS;

free_hal_data:
	if(status != _SUCCESS && padapter->HalData)
		rtw_mfree(padapter->HalData, sizeof(*(padapter->HalData)));

free_wdev:
	if(status != _SUCCESS) {
		#ifdef CONFIG_IOCTL_CFG80211
		rtw_wdev_unregister(padapter->rtw_wdev);
		rtw_wdev_free(padapter->rtw_wdev);
		#endif
	}

free_adapter:
	if (status != _SUCCESS) {
		if (pnetdev)
			rtw_free_netdev(pnetdev);
		else if (padapter)
			rtw_vmfree((u8*)padapter, sizeof(*padapter));
		padapter = NULL;
	}
exit:
	return padapter;
}

static void rtw_sdio_if1_deinit(_adapter *if1)
{
	struct net_device *pnetdev = if1->pnetdev;
	struct mlme_priv *pmlmepriv= &if1->mlmepriv;

#if 0
#if defined(CONFIG_HAS_EARLYSUSPEND ) || defined(CONFIG_ANDROID_POWER)
	rtw_unregister_early_suspend(adapter_to_pwrctl(if1));
#endif

	if (if1->bSurpriseRemoved == _FALSE)
	{
		struct sdio_func *func = adapter_to_dvobj(if1)->intf_data.func;

		// test surprise remove
		int err;

		sdio_claim_host(func);
		sdio_readb(func, 0, &err);
		sdio_release_host(func);
		if (err == -ENOMEDIUM) {
			if1->bSurpriseRemoved = _TRUE;
			DBG_871X(KERN_NOTICE "%s: device had been removed!\n", __func__);
		}
	}

	rtw_pm_set_ips(if1, IPS_NONE);
	rtw_pm_set_lps(if1, PS_MODE_ACTIVE);

	LeaveAllPowerSaveMode(if1);
#endif

	if(check_fwstate(pmlmepriv, _FW_LINKED))
		disconnect_hdl(if1, NULL);

#ifdef CONFIG_AP_MODE
	//free_mlme_ap_info(if1);
	#ifdef CONFIG_HOSTAPD_MLME
	hostapd_mode_unload(if1);
	#endif
#endif

	if(if1->DriverState != DRIVER_DISAPPEAR) {
		if(pnetdev) {
			unregister_netdev(pnetdev); //will call netdev_close()
			rtw_proc_remove_one(pnetdev);
		}
	}

	rtw_cancel_all_timer(if1);

#ifdef CONFIG_WOWLAN
	adapter_to_pwrctl(if1)->wowlan_mode=_FALSE;
	DBG_871X("%s wowlan_mode:%d\n", __func__, adapter_to_pwrctl(if1)->wowlan_mode);
#endif //CONFIG_WOWLAN

	rtw_dev_unload(if1);
	DBG_871X("+r871xu_dev_remove, hw_init_completed=%d\n", if1->hw_init_completed);

	rtw_handle_dualmac(if1, 0);

#ifdef CONFIG_AP_MODE
	free_mlme_ap_info(if1);
#endif

#ifdef CONFIG_IOCTL_CFG80211
	rtw_wdev_unregister(if1->rtw_wdev);
	rtw_wdev_free(if1->rtw_wdev);
#endif

	rtw_free_drv_sw(if1);

	if(pnetdev)
		rtw_free_netdev(pnetdev);

#ifdef CONFIG_PLATFORM_RTD2880B
	DBG_871X("wlan link down\n");
	rtd2885_wlan_netlink_sendMsg("linkdown", "8712");
#endif

}

#if (!(defined ANDROID_2X) && (defined CONFIG_PLATFORM_SPRD))
#if defined(CONFIG_PM_RUNTIME)
#include <linux/pm_runtime.h>
static void rtw_start_runtime(struct sdio_func *func)
{
	unsigned long flags;
	struct mmc_card *card = func->card;
	struct mmc_host *host = card->host;

	DBG_871X("%s \n", __func__);

	pm_runtime_no_callbacks(&func->dev);
	pm_suspend_ignore_children(&func->dev, true);
	pm_runtime_set_autosuspend_delay(&func->dev, 100);
	pm_runtime_use_autosuspend(&func->dev);
	pm_runtime_put_autosuspend(&func->dev);
}
#endif // CONFIG_PM_RUNTIME
#endif // (!(defined ANDROID_2X) && (defined CONFIG_PLATFORM_SPRD))

/*
 * drv_init() - a device potentially for us
 *
 * notes: drv_init() is called when the bus driver has located a card for us to support.
 *        We accept the new device by returning 0.
 */
static int rtw_drv_init(
	struct sdio_func *func,
	const struct sdio_device_id *id)
{
	int status = _FAIL;
	struct net_device *pnetdev;
	PADAPTER if1 = NULL, if2 = NULL;
	struct dvobj_priv *dvobj;

	RT_TRACE(_module_hci_intfs_c_, _drv_info_,
		("+rtw_drv_init: vendor=0x%04x device=0x%04x class=0x%02x\n",
		func->vendor, func->device, func->class));

	if ((dvobj = sdio_dvobj_init(func)) == NULL) {
		RT_TRACE(_module_hci_intfs_c_, _drv_err_, ("initialize device object priv Failed!\n"));
		goto exit;
	}

	if ((if1 = rtw_sdio_if1_init(dvobj, id)) == NULL) {
		DBG_871X("rtw_init_primary_adapter Failed!\n");
		goto free_dvobj;
	}

#ifdef CONFIG_CONCURRENT_MODE
	if ((if2 = rtw_drv_if2_init(if1, sdio_set_intf_ops)) == NULL) {
		goto free_if1;
	}
#endif

	//dev_alloc_name && register_netdev
	if((status = rtw_drv_register_netdev(if1)) != _SUCCESS) {
		goto free_if2;
	}

	if (sdio_alloc_irq(dvobj) != _SUCCESS)
		goto free_if2;

#ifdef CONFIG_GLOBAL_UI_PID
	if(ui_pid[1]!=0) {
		DBG_871X("ui_pid[1]:%d\n",ui_pid[1]);
		rtw_signal_process(ui_pid[1], SIGUSR2);
	}
#endif

#if (!(defined ANDROID_2X) && (defined CONFIG_PLATFORM_SPRD))
#if defined(CONFIG_PM_RUNTIME)
	//rtw_start_runtime(func);
#endif // CONFIG_PM_RUNTIME
#endif // (!(defined ANDROID_2X) && (defined CONFIG_PLATFORM_SPRD))
	RT_TRACE(_module_hci_intfs_c_,_drv_err_,("-871x_drv - drv_init, success!\n"));

	status = _SUCCESS;

free_if2:
	if(status != _SUCCESS && if2) {
		#ifdef CONFIG_CONCURRENT_MODE
		rtw_drv_if2_stop(if2);
		rtw_drv_if2_free(if2);
		#endif
	}
free_if1:
	if (status != _SUCCESS && if1) {
		rtw_sdio_if1_deinit(if1);
	}
free_dvobj:
	if (status != _SUCCESS)
		sdio_dvobj_deinit(func);
exit:
	return status == _SUCCESS?0:-ENODEV;
}

static void rtw_dev_remove(struct sdio_func *func)
{
	struct dvobj_priv *dvobj = sdio_get_drvdata(func);
	struct pwrctrl_priv *pwrctl = dvobj_to_pwrctl(dvobj);
	PADAPTER padapter = dvobj->if1;

_func_enter_;

	RT_TRACE(_module_hci_intfs_c_, _drv_notice_, ("+rtw_dev_remove\n"));

	if (padapter->bSurpriseRemoved == _FALSE) {
		int err;

		/* test surprise remove */
		sdio_claim_host(func);
		sdio_readb(func, 0, &err);
		sdio_release_host(func);
		if (err == -ENOMEDIUM) {
			padapter->bSurpriseRemoved = _TRUE;
			DBG_871X(KERN_NOTICE "%s: device had been removed!\n", __func__);
		}
	}

#if defined(CONFIG_HAS_EARLYSUSPEND) || defined(CONFIG_ANDROID_POWER)
	rtw_unregister_early_suspend(pwrctl);
#endif

	rtw_pm_set_ips(padapter, IPS_NONE);
	rtw_pm_set_lps(padapter, PS_MODE_ACTIVE);

	LeaveAllPowerSaveMode(padapter);

#ifdef CONFIG_CONCURRENT_MODE
	rtw_drv_if2_stop(dvobj->if2);
#endif

	rtw_sdio_if1_deinit(padapter);


#ifdef CONFIG_CONCURRENT_MODE
	rtw_drv_if2_free(dvobj->if2);
#endif


	sdio_dvobj_deinit(func);

	RT_TRACE(_module_hci_intfs_c_, _drv_notice_, ("-rtw_dev_remove\n"));

_func_exit_;
}

#ifndef CONFIG_SUSPEND_NEW_FLOW
static int rtw_sdio_suspend(struct device *dev)
{
	struct sdio_func *func =dev_to_sdio_func(dev);
	struct dvobj_priv *psdpriv = sdio_get_drvdata(func);
	_adapter *padapter = psdpriv->if1;
	struct pwrctrl_priv *pwrpriv = adapter_to_pwrctl(padapter);
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;
	struct net_device *pnetdev = padapter->pnetdev;
	int ret = 0;
#ifdef CONFIG_PLATFORM_SPRD
	u32 value;
#endif // CONFIG_PLATFORM_SPRD

#ifdef CONFIG_WOWLAN
	struct wowlan_ioctl_param poidparam;
	u8 ps_mode;
#endif //CONFIG_WOWLAN

	u32 start_time = rtw_get_current_time();

	_func_enter_;

	DBG_871X_LEVEL(_drv_always_, "sdio suspend start\n");
	DBG_871X("==> %s (%s:%d)\n",__FUNCTION__, current->comm, current->pid);

#if (!(defined ANDROID_2X) && (defined CONFIG_PLATFORM_SPRD))
	if(check_fwstate(pmlmepriv, WIFI_AP_STATE) == _TRUE) {
		//we should not suspend softap, bcm also do like this
		DBG_871X("%s should not suspend hw and system in AP mode\n",__FUNCTION__);
		goto exit;
	}
#endif

#ifdef CONFIG_WOWLAN
	if (check_fwstate(pmlmepriv, _FW_LINKED))
		pwrpriv->wowlan_mode = _TRUE;
	else
		pwrpriv->wowlan_mode = _FALSE;
#endif

	//pwrpriv->bInSuspend = _TRUE;

	while (pwrpriv->bips_processing == _TRUE)
		rtw_msleep_os(1);

	if((!padapter->bup) || (padapter->bDriverStopped)||(padapter->bSurpriseRemoved))
	{
		DBG_871X("%s bup=%d bDriverStopped=%d bSurpriseRemoved = %d\n", __FUNCTION__
			,padapter->bup, padapter->bDriverStopped,padapter->bSurpriseRemoved);
		goto exit;
	}

	rtw_cancel_all_timer(padapter);

	LeaveAllPowerSaveMode(padapter);
	//for power down during suspend, need leave ips mode before entering power down.
	pwrpriv->bInSuspend = _TRUE;
	//pwrpriv->do_late_resume = _FALSE;

	//padapter->net_closed = _TRUE;
	//s1.
#ifdef CONFIG_WOWLAN
	if(pnetdev && !pwrpriv->wowlan_mode)
	{
		netif_carrier_off(pnetdev);
		rtw_netif_stop_queue(pnetdev);
	} else if(pnetdev){
		DBG_871X("wowlan_mode: NO netif_carrier_off\n");
		rtw_netif_stop_queue(pnetdev);
	}
#else
	if(pnetdev)
	{
		netif_carrier_off(pnetdev);
		rtw_netif_stop_queue(pnetdev);
	}
#endif
#ifdef CONFIG_WOWLAN
	DBG_871X("wowlan_mode: %d\n", pwrpriv->wowlan_mode);
 	if ((pwrpriv->bSupportRemoteWakeup == _TRUE) &&
		(pwrpriv->wowlan_mode == _TRUE))
	{
		poidparam.subcode = WOWLAN_ENABLE;
		padapter->HalFunc.SetHwRegHandler(padapter,HW_VAR_WOWLAN,(u8 *)&poidparam);
	}
	else
#endif // CONFIG_WOWLAN
	{
		//s2.
		//s2-1.  issue rtw_disassoc_cmd to fw
		disconnect_hdl(padapter, NULL);
		//rtw_disassoc_cmd(padapter);
	}

	/* WOWLAN dont need roaming */
#ifndef CONFIG_WOWLAN
#ifdef CONFIG_LAYER2_ROAMING_RESUME
	if(check_fwstate(pmlmepriv, WIFI_STATION_STATE) && check_fwstate(pmlmepriv, _FW_LINKED) )
	{
		DBG_871X("%s %s(" MAC_FMT "), length:%d assoc_ssid.length:%d\n",__FUNCTION__,
				pmlmepriv->cur_network.network.Ssid.Ssid,
				MAC_ARG(pmlmepriv->cur_network.network.MacAddress),
				pmlmepriv->cur_network.network.Ssid.SsidLength,
				pmlmepriv->assoc_ssid.SsidLength);

		pmlmepriv->to_roaming = 1;
	}
#endif
#endif

#ifdef CONFIG_WOWLAN
	if (!pwrpriv->wowlan_mode)
	{
		//s2-2.  indicate disconnect to os
		rtw_indicate_disconnect(padapter, 0);
		//s2-3.
		rtw_free_assoc_resources(padapter, 1);

		//s2-4.
		rtw_free_network_queue(padapter, _FALSE);

		rtw_led_control(padapter, LED_CTL_POWER_OFF);

		rtw_dev_unload(padapter);

		if(check_fwstate(pmlmepriv, _FW_UNDER_SURVEY)){
			DBG_871X("%s: fw_under_survey\n", __FUNCTION__);
			rtw_indicate_scan_done(padapter, 1);
		}

		if(check_fwstate(pmlmepriv, _FW_UNDER_LINKING)){
			DBG_871X("%s: fw_under_linking\n", __FUNCTION__);
			rtw_indicate_disconnect(padapter, 0);
		}

		sdio_deinit(adapter_to_dvobj(padapter));
	}
	else
	{
		DBG_871X("%s: wowmode suspending\n", __FUNCTION__);

		DBG_871X( "%s: Before stop thread, DriverStopped=%d\n",
			__FUNCTION__, padapter->bDriverStopped);

		rtw_hal_disable_interrupt(padapter); // It need wait for leaving 32K.
		rtw_hal_clear_interrupt(padapter);

		padapter->bDriverStopped = _TRUE;       //for stop thread

		rtw_stop_drv_threads(padapter);

		padapter->bDriverStopped = _FALSE;      //for 32k command

		DBG_871X("%s: After stop thread, DriverStopped=%d\n",
			__FUNCTION__, padapter->bDriverStopped);

		if(check_fwstate(pmlmepriv, _FW_UNDER_SURVEY)){
			DBG_871X("%s: fw_under_survey\n", __FUNCTION__);
			rtw_indicate_scan_done(padapter, 1);
		}
		if(check_fwstate(pmlmepriv, _FW_UNDER_LINKING)){
			DBG_871X("%s: fw_under_linking\n", __FUNCTION__);
			rtw_indicate_disconnect(padapter, 0);
		}

#ifdef CONFIG_RTL8188E
#ifdef CONFIG_LPS
		rtw_set_ps_mode(padapter, PS_MODE_MIN, 0, 0);
#endif
		sdio_free_irq(adapter_to_dvobj(padapter));

		rtw_hal_disable_interrupt_but_cpwm2(padapter);
#else //CONFIG_RTL8188E
#ifdef CONFIG_LPS
#if defined(CONFIG_RTL8723B) || defined(CONFIG_RTL8188E)
		rtw_hal_set_hwreg(padapter, HW_VAR_H2C_PS_TUNE_PARAM, NULL);
#endif
		rtw_set_ps_mode(padapter, PS_MODE_SELF_DEFINED, 2, 0);
#endif
		sdio_free_irq(adapter_to_dvobj(padapter));

		rtw_hal_disable_interrupt_but_cpwm2(padapter);

		rtw_hal_get_hwreg(padapter, HW_VAR_WAKEUP_REASON, &pwrpriv->wowlan_wake_reason);
 		DBG_871X("%s(): Before sleep wowlan_wake_reason=0x%x  REG[0x287]=0x%x!!\n",__FUNCTION__,pwrpriv->wowlan_wake_reason, rtw_read8(padapter, 0x287));
		if(pwrpriv->wowlan_wake_reason != 0)
		{
 			DBG_871X("%s(): lock suspend 2s because wowlan_wake_reason=0x%x!!\n",__FUNCTION__,pwrpriv->wowlan_wake_reason);
         		rtw_lock_suspend_timeout(2 * HZ);
		}
#ifdef CONFIG_GPIO_WAKEUP
		gpio_hostwakeup_alloc_irq(padapter);
#endif
#endif //CONFIG_RTL8188E
	}
#else
	//s2-2.  indicate disconnect to os
	rtw_indicate_disconnect(padapter, 0);
	//s2-3.
	rtw_free_assoc_resources(padapter, 1);

	//s2-4.
	rtw_free_network_queue(padapter, _FALSE);

	rtw_led_control(padapter, LED_CTL_POWER_OFF);

	rtw_dev_unload(padapter);

	if(check_fwstate(pmlmepriv, _FW_UNDER_SURVEY))
		rtw_indicate_scan_done(padapter, 1);

	if(check_fwstate(pmlmepriv, _FW_UNDER_LINKING))
		rtw_indicate_disconnect(padapter, 0);

	// interface deinit
	sdio_deinit(adapter_to_dvobj(padapter));
#endif
	DBG_871X("sdio suspend success in %d ms\n",
			rtw_get_passing_time_ms(start_time));

exit:

#if (defined CONFIG_WOWLAN) || (defined CONFIG_PLATFORM_SPRD)
	//Android 4.0 don't support WIFI close power
	//or power down or clock will close after wifi resume,
	//this is sprd's bug in Android 4.0, but sprd don't
	//want to fix it.
	//we have test power under 8723as, power consumption is ok
	if (func)
	{
		mmc_pm_flag_t pm_flag = 0;
		pm_flag = sdio_get_host_pm_caps(func);
		DBG_871X("cmd: %s: suspend: PM flag = 0x%x\n", sdio_func_id(func), pm_flag);

		if (!(pm_flag & MMC_PM_KEEP_POWER))
		{
			DBG_871X("%s: cannot remain alive while host is suspended\n", sdio_func_id(func));
#ifdef CONFIG_PLATFORM_HUAWEI_K3V2
			pm_flag |= MMC_PM_KEEP_POWER;
			sdio_set_host_pm_flags(func, pm_flag);
#else
			return -ENOSYS;
#endif
		}
		else
		{
			DBG_871X("cmd: suspend with MMC_PM_KEEP_POWER\n");
			pm_flag |= MMC_PM_KEEP_POWER;
			sdio_set_host_pm_flags(func, pm_flag);
		}
	}
#endif

#ifdef CONFIG_PLATFORM_SPRD
#ifndef CONFIG_WOWLAN
#ifdef CONFIG_RTL8188E
#ifdef ANDROID_2X
	/*
	 * Pull down wifi power pin here
	 * Pull up wifi power pin before sdio resume.
	 */
	rtw_wifi_gpio_wlan_ctrl(WLAN_POWER_OFF);
#endif // ANDROID_2X
#endif // CONFIG_RTL8188E
#endif // CONFIG_WOWLAN
#endif // CONFIG_PLATFORM_SPRD

	DBG_871X("<===  %s return %d.............. in %dms\n", __FUNCTION__
		, ret, rtw_get_passing_time_ms(start_time));

	_func_exit_;
	return ret;
}
#else

static int rtw_sdio_suspend(struct device *dev)
{
	struct sdio_func *func =dev_to_sdio_func(dev);
	struct dvobj_priv *psdpriv = sdio_get_drvdata(func);
	_adapter *padapter = psdpriv->if1;
	struct pwrctrl_priv *pwrpriv = adapter_to_pwrctl(padapter);
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;
	struct net_device *pnetdev = padapter->pnetdev;
	int ret = 0;
#ifdef CONFIG_WOWLAN
	u8 ps_mode;
#endif //CONFIG_WOWLAN
	u32 start_time = rtw_get_current_time();

	_func_enter_;

	DBG_871X_LEVEL(_drv_always_, "sdio suspend start\n");
//	DBG_871X("==> %s (%s:%d)\n",__FUNCTION__, current->comm, current->pid);

#if (!(defined ANDROID_2X) && (defined CONFIG_PLATFORM_SPRD))
	if(check_fwstate(pmlmepriv, WIFI_AP_STATE) == _TRUE) {
		//we should not suspend softap, bcm also do like this
		DBG_871X("%s should not suspend hw and system in AP mode\n",__FUNCTION__);
		goto exit;
	}
#endif

	if((!padapter->bup) || (padapter->bDriverStopped)||(padapter->bSurpriseRemoved))
	{
		DBG_871X("%s bup=%d bDriverStopped=%d bSurpriseRemoved = %d\n", __FUNCTION__
			,padapter->bup, padapter->bDriverStopped,padapter->bSurpriseRemoved);
		goto exit;
	}


	DBG_871X("%s: pmlmepriv->fw_state=0x%x bSupportRemoteWakeup=%d\n", __FUNCTION__, pmlmepriv->fw_state, pwrpriv->bSupportRemoteWakeup);
	/* linked suspend->ap power off/on->resume disconnect-> */
	/* nolinked suspend->resume->auto linked->suspend/resume */
	/* will call resume_process because do_late_resume is wrong */
	//pwrpriv->do_late_resume = _FALSE;
#ifdef CONFIG_WOWLAN
	if (check_fwstate(pmlmepriv, _FW_LINKED) && (pwrpriv->bSupportRemoteWakeup == _TRUE))
		pwrpriv->wowlan_mode = _TRUE;
	else
		pwrpriv->wowlan_mode = _FALSE;

	if(pwrpriv->wowlan_mode)
	{
		struct wowlan_ioctl_param poidparam;

		DBG_871X("%s: wowmode suspending\n", __FUNCTION__);

		// 1. Cancel all timer
		rtw_cancel_all_timer(padapter);

		// 2. Leave LPS
#ifdef CONFIG_RTL8188E
		rtw_set_ps_mode(padapter, PS_MODE_ACTIVE, 0, 0);
#else
		LeaveAllPowerSaveMode(padapter);
#endif
		pwrpriv->bInSuspend = _TRUE;

		// 3. Stop thread
		rtw_hal_disable_interrupt(padapter);
		rtw_hal_clear_interrupt(padapter);

		padapter->bDriverStopped = _TRUE;	//for stop thread. must disable rx interrupt when bDriverStopped is set to TRUE.
		rtw_stop_drv_threads(padapter);
		padapter->bDriverStopped = _FALSE;

		rtw_hal_enable_interrupt(padapter);

		// 4. Enable WoWlan
		poidparam.subcode = WOWLAN_ENABLE;
		padapter->HalFunc.SetHwRegHandler(padapter,HW_VAR_WOWLAN,(u8 *)&poidparam);

		if(check_fwstate(pmlmepriv, _FW_UNDER_SURVEY)){
			DBG_871X("%s: fw_under_survey\n", __FUNCTION__);
			rtw_indicate_scan_done(padapter, 1);
		}

		// 5. Disable interrupt
		rtw_hal_disable_interrupt(padapter);
		rtw_hal_clear_interrupt(padapter);

		// 6. Free irq
		sdio_free_irq(adapter_to_dvobj(padapter));

		// 7. Enter LPS
#ifdef CONFIG_LPS
#if defined(CONFIG_RTL8723B) || defined(CONFIG_RTL8188E)
		rtw_hal_set_hwreg(padapter, HW_VAR_H2C_PS_TUNE_PARAM, NULL);
#endif
		rtw_set_ps_mode(padapter, PS_MODE_SELF_DEFINED, 2, 0);
#endif

		// 8. Enable CPWM2 interrupt
		rtw_hal_disable_interrupt_but_cpwm2(padapter);

		// 9. Check if FW wakeup host is triggered.
		rtw_hal_get_hwreg(padapter, HW_VAR_WAKEUP_REASON, &pwrpriv->wowlan_wake_reason);
		if(pwrpriv->wowlan_wake_reason != 0)
		{
 			DBG_871X("%s(): lock suspend 2s because wowlan_wake_reason=0x%x!!\n",__FUNCTION__,pwrpriv->wowlan_wake_reason);
         		rtw_lock_suspend_timeout(2 * HZ);
		}
#ifdef CONFIG_GPIO_WAKEUP
		gpio_hostwakeup_alloc_irq(padapter);
#endif
	}
	else
#endif
	{
		while (pwrpriv->bips_processing == _TRUE)
			rtw_msleep_os(1);

		// 1. Cancel timer
		rtw_cancel_all_timer(padapter);

		pwrpriv->bInSuspend = _TRUE;

#ifndef CONFIG_WOWLAN
		//s1.
		if(pnetdev)
		{
			netif_carrier_off(pnetdev);
			rtw_netif_stop_queue(pnetdev);
		}

		//s2.
		//s2-1.  issue rtw_disassoc_cmd to fw
		disconnect_hdl(padapter, NULL);

#ifdef CONFIG_LAYER2_ROAMING_RESUME
		if(check_fwstate(pmlmepriv, WIFI_STATION_STATE) && check_fwstate(pmlmepriv, _FW_LINKED) )
		{
			DBG_871X("%s %s(" MAC_FMT "), length:%d assoc_ssid.length:%d\n",__FUNCTION__,
					pmlmepriv->cur_network.network.Ssid.Ssid,
					MAC_ARG(pmlmepriv->cur_network.network.MacAddress),
					pmlmepriv->cur_network.network.Ssid.SsidLength,
					pmlmepriv->assoc_ssid.SsidLength);

			pmlmepriv->to_roaming = 1;
		}
#endif

		//s2-2.  indicate disconnect to os
		rtw_indicate_disconnect(padapter, 0);
		//s2-3.
		rtw_free_assoc_resources(padapter, 1);

		//s2-4.
		rtw_free_network_queue(padapter, _FALSE);
#endif

		// 2. Check Fw state
		if(check_fwstate(pmlmepriv, _FW_UNDER_SURVEY)){
			DBG_871X("%s: fw_under_survey\n", __FUNCTION__);
			//rtw_indicate_scan_done(padapter, 1);
			rtw_scan_abort(padapter);
		}

		if(check_fwstate(pmlmepriv, _FW_UNDER_LINKING)){
			DBG_871X("%s: fw_under_linking\n", __FUNCTION__);
			rtw_indicate_disconnect(padapter, 0);
		}

		// 3. Stop thread
		rtw_hal_disable_interrupt(padapter);
		rtw_hal_clear_interrupt(padapter);
		padapter->bDriverStopped = _TRUE;	//for stop thread. must disable rx interrupt when bDriverStopped is set to TRUE.
		rtw_stop_drv_threads(padapter);
		padapter->bDriverStopped = _FALSE;

		// 4. Enter IPS
#ifdef CONFIG_IPS
		if(pwrpriv->rf_pwrstate == rf_on)
		{
			pwrpriv->change_rfpwrstate = rf_off;

			/* clean fifo need it, ips will close it */
			rtw_hal_enable_interrupt(padapter);
			ips_enter(padapter);
		}
#endif
		// 5. Free and release IRQ
		sdio_free_irq(adapter_to_dvobj(padapter));
		sdio_deinit(adapter_to_dvobj(padapter));
	}

	DBG_871X("sdio suspend success in %d ms\n",
			rtw_get_passing_time_ms(start_time));

exit:

#if (defined CONFIG_WOWLAN) || (defined CONFIG_PLATFORM_SPRD)
	//Android 4.0 don't support WIFI close power
	//or power down or clock will close after wifi resume,
	//this is sprd's bug in Android 4.0, but sprd don't
	//want to fix it.
	//we have test power under 8723as, power consumption is ok
	if (func)
	{
		mmc_pm_flag_t pm_flag = 0;
		pm_flag = sdio_get_host_pm_caps(func);
		DBG_871X("cmd: %s: suspend: PM flag = 0x%x\n", sdio_func_id(func), pm_flag);

		if (!(pm_flag & MMC_PM_KEEP_POWER))
		{
			DBG_871X("%s: cannot remain alive while host is suspended\n", sdio_func_id(func));
#ifdef CONFIG_PLATFORM_HUAWEI_K3V2
			pm_flag |= MMC_PM_KEEP_POWER;
			sdio_set_host_pm_flags(func, pm_flag);
#else
			return -ENOSYS;
#endif
		}
		else
		{
			DBG_871X("cmd: suspend with MMC_PM_KEEP_POWER\n");
			pm_flag |= MMC_PM_KEEP_POWER;
			sdio_set_host_pm_flags(func, pm_flag);
		}
	}
#endif

#if (defined ANDROID_2X) && (defined CONFIG_PLATFORM_SPRD)
#ifndef CONFIG_WOWLAN
#ifdef CONFIG_RTL8188E
	/*
	 * Pull down wifi power pin here
	 * Pull up wifi power pin before sdio resume.
	 */
	rtw_wifi_gpio_wlan_ctrl(WLAN_POWER_OFF);
#endif // CONFIG_RTL8188E
#endif // CONFIG_WOWLAN
#endif // CONFIG_PLATFORM_SPRD && ANDROID_2X

	DBG_871X("<===  %s return %d.............. in %dms\n", __FUNCTION__
		, ret, rtw_get_passing_time_ms(start_time));


	pwrpriv->bInSuspend = _FALSE;

	_func_exit_;

	return ret;
}
#endif

extern int pm_netdev_open(struct net_device *pnetdev,u8 bnormal);

#ifndef CONFIG_SUSPEND_NEW_FLOW
int rtw_resume_process(_adapter *padapter)
{
	struct net_device *pnetdev;
	struct pwrctrl_priv *pwrpriv;
	u8 is_pwrlock_hold_by_caller;
	u8 is_directly_called_by_auto_resume;
	int ret = 0;
	u32 start_time = rtw_get_current_time();
#ifdef CONFIG_WOWLAN
	u32 value = 0;
        struct wowlan_ioctl_param poidparam;
#endif // CONFIG_WOWLAN

	_func_enter_;

	DBG_871X_LEVEL(_drv_always_, "sdio resume start\n");
	DBG_871X("==> %s (%s:%d)\n",__FUNCTION__, current->comm, current->pid);
	//DBG_871X("REG[0x1B8]=%.8x\n", rtw_read32(padapter, 0x1B8));

	if (padapter) {
		pnetdev = padapter->pnetdev;
		pwrpriv = adapter_to_pwrctl(padapter);
	} else {
		ret = -1;
		goto exit;
	}

#ifdef CONFIG_WOWLAN
	if (!pwrpriv->wowlan_mode)
	{
		// interface init
		if (sdio_init(adapter_to_dvobj(padapter)) != _SUCCESS)
		{
			ret = -1;
			RT_TRACE(_module_hci_intfs_c_, _drv_err_, ("%s: initialize SDIO Failed!!\n", __FUNCTION__));
			goto exit;
		}
		rtw_hal_disable_interrupt(padapter);
		if (sdio_alloc_irq(adapter_to_dvobj(padapter)) != _SUCCESS)
		{
			ret = -1;
			RT_TRACE(_module_hci_intfs_c_, _drv_err_, ("%s: sdio_alloc_irq Failed!!\n", __FUNCTION__));
			goto exit;
		}

		rtw_reset_drv_sw(padapter);
		pwrpriv->bkeepfwalive = _FALSE;

		DBG_871X("bkeepfwalive(%x)\n",pwrpriv->bkeepfwalive);
		if(pm_netdev_open(pnetdev,_TRUE) != 0) {
			ret = -1;
			goto exit;
		}

		netif_device_attach(pnetdev);
		netif_carrier_on(pnetdev);
	}
	else
	{
#ifdef CONFIG_GPIO_WAKEUP
		gpio_hostwakeup_free_irq(padapter);
#endif
#ifdef CONFIG_LPS
		rtw_set_ps_mode(padapter, PS_MODE_ACTIVE, 0, 0);
		LPS_RF_ON_check(padapter, 100);
#endif //CONFIG_LPS

		rtw_hal_disable_interrupt(padapter);
		if (sdio_alloc_irq(adapter_to_dvobj(padapter)) != _SUCCESS) {
			ret = -1;
			RT_TRACE(_module_hci_intfs_c_, _drv_err_, ("%s: sdio_alloc_irq Failed!!\n", __FUNCTION__));
			goto exit;
		}

		rtw_hal_clear_interrupt(padapter);

		rtw_hal_enable_interrupt(padapter);

		padapter->bDriverStopped = _FALSE;
		DBG_871X("%s: wowmode resuming, DriverStopped:%d\n", __func__, padapter->bDriverStopped);
		rtw_start_drv_threads(padapter);

		pwrpriv->wowlan_mode = _FALSE;

		//Disable WOW, set H2C command
		poidparam.subcode=WOWLAN_DISABLE;
		padapter->HalFunc.SetHwRegHandler(padapter,HW_VAR_WOWLAN,(u8 *)&poidparam);

		// start netif queue
		if(pnetdev) {
			if(!rtw_netif_queue_stopped(pnetdev))
				rtw_netif_start_queue(pnetdev);
			else
				rtw_netif_wake_queue(pnetdev);
		}
	}
#else // !CONFIG_WOWLAN
	// interface init
	if (sdio_init(adapter_to_dvobj(padapter)) != _SUCCESS)
	{
		ret = -1;
		RT_TRACE(_module_hci_intfs_c_, _drv_err_, ("%s: initialize SDIO Failed!!\n", __FUNCTION__));
		goto exit;
	}
	rtw_hal_disable_interrupt(padapter);
	if (sdio_alloc_irq(adapter_to_dvobj(padapter)) != _SUCCESS)
	{
		ret = -1;
		RT_TRACE(_module_hci_intfs_c_, _drv_err_, ("%s: sdio_alloc_irq Failed!!\n", __FUNCTION__));
		goto exit;
	}

	rtw_reset_drv_sw(padapter);
	pwrpriv->bkeepfwalive = _FALSE;

	DBG_871X("bkeepfwalive(%x)\n",pwrpriv->bkeepfwalive);
	if(pm_netdev_open(pnetdev,_TRUE) != 0) {
		ret = -1;
		goto exit;
	}

	netif_device_attach(pnetdev);
	netif_carrier_on(pnetdev);
#endif
	if( padapter->pid[1]!=0) {
		DBG_871X("pid[1]:%d\n",padapter->pid[1]);
		rtw_signal_process(padapter->pid[1], SIGUSR2);
	}

#ifdef CONFIG_LAYER2_ROAMING_RESUME
#ifdef CONFIG_WOWLAN
	if(!check_fwstate(&(padapter->mlmepriv), _FW_LINKED))
	{
		rtw_roaming(padapter, NULL);
	} else if (pwrpriv->wowlan_wake_reason == FWDecisionDisconnect){
		rtw_indicate_disconnect(padapter, 0);
	} else if (pwrpriv->wowlan_wake_reason == Rx_GTK){
		rtw_roaming(padapter, NULL);
	}
#else
	rtw_roaming(padapter, NULL);
#endif //CONFOG_WOWLAN

	/* linked suspend set it to 1, if resume not set back to 0 */
	/* after we disconnect from UI, and suspend/resume will roaming */
	padapter->mlmepriv.to_roaming = 0;
#endif //CONFIG_LAYER2_ROAMING_RESUME

	#ifdef CONFIG_RESUME_IN_WORKQUEUE
	rtw_unlock_suspend();
	#endif //CONFIG_RESUME_IN_WORKQUEUE

	pwrpriv->bInSuspend = _FALSE;

#ifdef CONFIG_WOWLAN
#if 0
	if (padapter->pwrctrlpriv.wowlan_mode)
		rtw_pm_set_ips(padapter, IPS_NONE);
#endif

	_set_timer(&padapter->mlmepriv.dynamic_chk_timer, 2000);
	/* linked suspend/resume and then disconnect, IPS can not enter issue */
	rtw_set_pwr_state_check_timer(pwrpriv);

#endif //CONFIG_WOWLAN

exit:
	DBG_871X_LEVEL(_drv_always_, "sdio resume success in %d ms\n",
			rtw_get_passing_time_ms(start_time));

	_func_exit_;

	return ret;
}
#else

int rtw_resume_process(_adapter *padapter)
{
	struct net_device *pnetdev;
	struct pwrctrl_priv *pwrpriv;
	u8 is_pwrlock_hold_by_caller;
	u8 is_directly_called_by_auto_resume;
	int ret = 0;
	u32 start_time = rtw_get_current_time();

	_func_enter_;

	DBG_871X_LEVEL(_drv_always_, "sdio resume start\n");
	//DBG_871X("==> %s (%s:%d)\n",__FUNCTION__, current->comm, current->pid);
//	DBG_871X("REG[0x1B8]=%.8x\n", rtw_read32(padapter, 0x1B8));
	DBG_871X("REG[0x204]=%.8x\n", rtw_read32(padapter, 0x204));
	DBG_871X("REG[0x13F]=%.2x\n", rtw_read8(padapter, 0x13F));
	DBG_871X("REG[0x284]=%.8x\n", rtw_read32(padapter, 0x284));

	if (padapter) {
		pnetdev = padapter->pnetdev;
		pwrpriv = adapter_to_pwrctl(padapter);
	} else {
		ret = -1;
		goto exit;
	}

#ifdef CONFIG_WOWLAN
	if (pwrpriv->wowlan_mode)
	{
		struct wowlan_ioctl_param poidparam;

#ifdef CONFIG_GPIO_WAKEUP
		gpio_hostwakeup_free_irq(padapter);
#endif
		// 1. Leave LPS 32k
#ifdef CONFIG_LPS
		rtw_set_ps_mode(padapter, PS_MODE_ACTIVE, 0, 0);
		LPS_RF_ON_check(padapter, 100);
#endif //CONFIG_LPS

		// 2. Disable interrupt
		// 2.1. Disable interrupt
		rtw_hal_disable_interrupt(padapter);

		// 2.2. Clear interrupt
		rtw_hal_clear_interrupt(padapter);

		// 3. Start thread
		padapter->bDriverStopped = _FALSE;
		rtw_start_drv_threads(padapter);

		// 4. Enable interrupt
		// 4.1. Allocate IRQ
		if (sdio_alloc_irq(adapter_to_dvobj(padapter)) != _SUCCESS) {
			ret = -1;
			RT_TRACE(_module_hci_intfs_c_, _drv_err_, ("%s: sdio_alloc_irq Failed!!\n", __FUNCTION__));
			goto exit;
		}

		// 4.2. Enable interrupt
		rtw_hal_enable_interrupt(padapter);

		// 5. Disable WoWlan
		poidparam.subcode=WOWLAN_DISABLE;
		padapter->HalFunc.SetHwRegHandler(padapter,HW_VAR_WOWLAN,(u8 *)&poidparam);

		pwrpriv->wowlan_mode = _FALSE;

		// 6. Set dynamic check timer and power state check timer
		_set_timer(&padapter->mlmepriv.dynamic_chk_timer, 2000);
		pwrpriv->bips_processing = _FALSE;
		rtw_set_pwr_state_check_timer(pwrpriv);

		//7. Send sig
		if( padapter->pid[1]!=0) {
			DBG_871X("pid[1]:%d\n",padapter->pid[1]);
			rtw_signal_process(padapter->pid[1], SIGUSR2);
		}

		//8. Check wakeup reason
#ifdef CONFIG_LAYER2_ROAMING_RESUME
	 	if (pwrpriv->wowlan_wake_reason == FWDecisionDisconnect ||
			pwrpriv->wowlan_wake_reason == Rx_DisAssoc ||
			pwrpriv->wowlan_wake_reason == Rx_DeAuth) {
			DBG_871X("%s: disconnect reason: %02x\n", __func__,
							pwrpriv->wowlan_wake_reason);
			rtw_indicate_disconnect(padapter, 0);
			rtw_free_assoc_resources(padapter, 1);
		}
#endif //CONFIG_LAYER2_ROAMING_RESUME
	}
	else
#endif	//CONFIG_WOWLAN
	{
		DBG_871X("%s(): IPS mode. do nothing\n",__FUNCTION__);

#ifndef CONFIG_WOWLAN
		rtw_reset_drv_sw(padapter);
		pwrpriv->bkeepfwalive = _FALSE;

		DBG_871X("bkeepfwalive(%x)\n",pwrpriv->bkeepfwalive);
		if(pm_netdev_open(pnetdev, _TRUE) != 0) {
			ret = -1;
			goto exit;
		}

		netif_device_attach(pnetdev);
		netif_carrier_on(pnetdev);

		rtw_roaming(padapter, NULL);
#endif
		if( padapter->pid[1]!=0) {
			DBG_871X("pid[1]:%d\n",padapter->pid[1]);
			rtw_signal_process(padapter->pid[1], SIGUSR2);
		}
	}

#ifdef CONFIG_RESUME_IN_WORKQUEUE
	rtw_unlock_suspend();
#endif //CONFIG_RESUME_IN_WORKQUEUE

exit:
	DBG_871X_LEVEL(_drv_always_, "sdio resume success in %d ms\n",
			rtw_get_passing_time_ms(start_time));

	_func_exit_;

	return ret;
}
#endif

static int rtw_sdio_resume(struct device *dev)
{
	struct sdio_func *func =dev_to_sdio_func(dev);
	struct dvobj_priv *psdpriv = sdio_get_drvdata(func);
	_adapter *padapter = psdpriv->if1;
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;
	struct pwrctrl_priv *pwrpriv = adapter_to_pwrctl(padapter);
	 int ret = 0;

	DBG_871X("==> %s (%s:%d)\n",__FUNCTION__, current->comm, current->pid);
#if defined CONFIG_MMC_SUNXI_POWER_CONTROL
    unsigned int mod_sel = mmc_pm_get_mod_type();
	if(mod_sel == SUNXI_SDIO_WIFI_NUM_RTL8723BS) {
		printk("[rtl8723bs] %s: resume start.\n", __FUNCTION__);
	} else {
		printk("[rtl8723bs] %s: mod_sel = %d is incorrect./n", __FUNCTION__, mod_sel);
	}
#endif
#ifdef CONFIG_PLATFORM_HUAWEI_K3V2
	if (func)
	{
		mmc_pm_flag_t pm_flag = 0;
		pm_flag = sdio_get_host_pm_caps(func);
		DBG_871X("cmd: %s: suspend: PM flag = 0x%x\n", sdio_func_id(func), pm_flag);

		if (pm_flag & MMC_PM_KEEP_POWER)
		{
			pm_flag &= ~MMC_PM_KEEP_POWER;
			sdio_set_host_pm_flags(func, pm_flag);
		}
	}
#endif

	if (pwrpriv->bInternalAutoSuspend)
	{
 		ret = rtw_resume_process(padapter);
	}
	else
	{
#ifdef CONFIG_RESUME_IN_WORKQUEUE
		rtw_resume_in_workqueue(pwrpriv);
#elif defined(CONFIG_HAS_EARLYSUSPEND) || defined(CONFIG_ANDROID_POWER)
		if (rtw_is_earlysuspend_registered(pwrpriv)
#ifdef CONFIG_WOWLAN
			&& !pwrpriv->wowlan_mode
#endif //CONFIG_WOWLAN
			)
		{
#ifdef CONFIG_SUSPEND_NEW_FLOW
			DBG_871X("%s(): start thread\n",__FUNCTION__);
			rtw_start_drv_threads(padapter);

			DBG_871X("%s(): sdio init\n",__FUNCTION__);
			// interface init
			if (sdio_init(adapter_to_dvobj(padapter)) != _SUCCESS)
			{
				ret = -1;
				RT_TRACE(_module_hci_intfs_c_, _drv_err_, ("%s: initialize SDIO Failed!!\n", __FUNCTION__));
				goto exit;
			}
//			rtw_hal_disable_interrupt(padapter);
			if (sdio_alloc_irq(adapter_to_dvobj(padapter)) != _SUCCESS)
			{
				ret = -1;
				RT_TRACE(_module_hci_intfs_c_, _drv_err_, ("%s: sdio_alloc_irq Failed!!\n", __FUNCTION__));
				goto exit;
			}
#endif
			//jeff: bypass resume here, do in late_resume
			//pwrpriv->do_late_resume = _TRUE;
#if (!(defined ANDROID_2X) && (defined CONFIG_PLATFORM_SPRD))
			//we should not suspend/resume softap, bcm also do like this
			if(check_fwstate(pmlmepriv, WIFI_AP_STATE) == _TRUE)
				pwrpriv->do_late_resume = _FALSE;
#endif
		}
		else
		{
#ifdef CONFIG_WOWLAN
			rtw_lock_suspend_timeout(4 * HZ);
#endif
			ret = rtw_resume_process(padapter);
		}
#else // Normal resume process
		ret = rtw_resume_process(padapter);
#endif //CONFIG_RESUME_IN_WORKQUEUE
	}

#ifdef CONFIG_SUSPEND_NEW_FLOW
exit:
#endif
	DBG_871X("<========  %s return %d\n", __FUNCTION__, ret);
	return ret;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,24))
extern int console_suspend_enabled;
#endif

#ifdef CONFIG_PLATFORM_SPRD
extern void sdhci_bus_scan(void);
#ifndef ANDROID_2X
extern int sdhci_device_attached(void);
#endif
#endif // CONFIG_PLATFORM_SPRD

static int __init rtw_drv_entry(void)
{
	int ret = 0;

	DBG_871X_LEVEL(_drv_always_, "module init start version:"DRIVERVERSION"\n");
//	DBG_871X(KERN_INFO "+%s", __func__);
	RT_TRACE(_module_hci_intfs_c_, _drv_notice_, ("+rtw_drv_entry\n"));
	DBG_871X(DRV_NAME " driver version=%s\n", DRIVERVERSION);
	DBG_871X("build time: %s %s\n", __DATE__, __TIME__);

#ifdef CONFIG_PLATFORM_ARM_SUN4I
/*depends on sunxi power control */
#if defined CONFIG_MMC_SUNXI_POWER_CONTROL
	unsigned int mod_sel = mmc_pm_get_mod_type();
	if(mod_sel == SUNXI_SDIO_WIFI_NUM_RTL8189ES)
	{
		rtl8189es_sdio_powerup();
		sunximmc_rescan_card(SDIOID, 1);
		DBG_8192C("[rtl8189es] %s: power up, rescan card.\n", __FUNCTION__);
	}
	else if(mod_sel == SUNXI_SDIO_WIFI_NUM_RTL8723BS)
	{
		rtl8723bs_sdio_powerup();
		sunximmc_rescan_card(SDIOID, 1);
		DBG_8192C("[rtl8723bs] %s: power up, rescan card.\n", __FUNCTION__);
	}
	else
	{
		ret = -1;
		DBG_8192C("%s: mod_sel = %d is incorrect.\n", __FUNCTION__, mod_sel);
	}
#endif	// defined CONFIG_MMC_SUNXI_POWER_CONTROL
	if(ret != 0)
		goto exit;

#endif //CONFIG_PLATFORM_ARM_SUN4I

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,24))
	//console_suspend_enabled=0;
#endif

#ifdef CONFIG_PLATFORM_HUAWEI_K3V2
	rtw_wifi_gpio_init();
    rtw_android_wifictrl_func_add();
#endif

#ifdef PLATFORM_LEADCORE_1813
	rtw_wifi_gpio_init();
	rtw_android_wifictrl_func_add();
#endif


#ifdef CONFIG_PLATFORM_SPRD
	rtw_wifi_gpio_init();

#ifdef ANDROID_2X
#ifdef CONFIG_RTL8188E
	rtw_wifi_gpio_wlan_ctrl(WLAN_POWER_ON);
#endif //CONFIG_RTL8188E
#endif //ANDROID_2X

	/* Pull up pwd pin, make wifi leave power down mode. */
	rtw_wifi_gpio_wlan_ctrl(WLAN_PWDN_ON);
#ifndef CONFIG_DONT_BUS_SCAN
	rtw_mdelay_os(5);
	sdhci_bus_scan();

#if (defined ANDROID_2X)
	rtw_mdelay_os(200);
#else
	if (1) {
		int i = 0;

		for (i = 0; i <= 50; i++) {
			msleep(10);
			if (sdhci_device_attached())
				break;
			DBG_871X("%s delay times:%d\n", __func__, i);
		}
	}
	rtw_msleep_os(150);
#endif
#endif
#endif // CONFIG_PLATFORM_SPRD

	rtw_suspend_lock_init();

	sdio_drvpriv.drv_registered = _TRUE;

	ret = sdio_register_driver(&sdio_drvpriv.r871xs_drv);

exit:
	DBG_871X_LEVEL(_drv_always_, "module init ret=%d\n", ret);

#if (!defined CONFIG_PLATFORM_HUAWEI_K3V2) && (!defined PLATFORM_LEADCORE_1813)
	rtw_android_wifictrl_func_add();
#endif

	return ret;
}

static void __exit rtw_drv_halt(void)
{
	DBG_871X_LEVEL(_drv_always_, "module exit start\n");

#if (!defined CONFIG_PLATFORM_HUAWEI_K3V2) && (!defined PLATFORM_LEADCORE_1813)
	rtw_android_wifictrl_func_del();
#endif

	rtw_suspend_lock_uninit();
	sdio_drvpriv.drv_registered = _FALSE;

	sdio_unregister_driver(&sdio_drvpriv.r871xs_drv);

#ifdef CONFIG_PLATFORM_HUAWEI_K3V2
	rtw_android_wifictrl_func_del();
	rtw_mdelay_os(5);
	rtw_wifi_gpio_deinit();
#endif

#ifdef CONFIG_PLATFORM_SPRD
	/* Pull down pwd pin, make wifi enter power down mode. */
	rtw_wifi_gpio_wlan_ctrl(WLAN_PWDN_OFF);
	rtw_mdelay_os(5);

#ifdef ANDROID_2X
#ifdef CONFIG_RTL8188E
	rtw_wifi_gpio_wlan_ctrl(WLAN_POWER_OFF);
#endif // CONFIG_RTL8188E
#endif // ANDROID_2X

	rtw_wifi_gpio_deinit();

#ifndef CONFIG_DONT_BUS_SCAN
#if defined(CONFIG_RTL8188E) || defined(CONFIG_RTL8723B)
	// After power off, notify host to release sdio card
	sdhci_bus_scan();
#endif
#endif
#endif // CONFIG_PLATFORM_SPRD

#ifdef CONFIG_PLATFORM_ARM_SUN4I
#if defined(CONFIG_MMC_SUNXI_POWER_CONTROL)
	sunximmc_rescan_card(SDIOID, 0);
#ifdef CONFIG_RTL8188E
	rtl8189es_sdio_poweroff();
	DBG_8192C("[rtl8189es] %s: remove card, power off.\n", __FUNCTION__);
#endif //CONFIG_RTL8188E
#ifdef CONFIG_RTL8723B
	rtl8723bs_sdio_poweroff();
	DBG_8192C("[rtl8723bs] %s: remove card, power off.\n", __FUNCTION__);
#endif //CONFIG_RTL8188E
#endif //defined(CONFIG_MMC_SUNXI_POWER_CONTROL)
#endif //CONFIG_PLATFORM_ARM_SUN4I

#ifdef PLATFORM_LEADCORE_1813
		rtw_android_wifictrl_func_del();
		rtw_mdelay_os(5);
		rtw_wifi_gpio_deinit();
#endif

	DBG_871X_LEVEL(_drv_always_, "module exit success\n");
}


module_init(rtw_drv_entry);
module_exit(rtw_drv_halt);

