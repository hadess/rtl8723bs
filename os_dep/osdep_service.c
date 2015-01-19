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


#define _OSDEP_SERVICE_C_

#include <drv_types.h>

/*
* Translate the OS dependent @param error_code to OS independent RTW_STATUS_CODE
* @return: one of RTW_STATUS_CODE
*/
inline int RTW_STATUS_CODE(int error_code){
	if(error_code >=0)
		return _SUCCESS;

	switch(error_code) {
		//case -ETIMEDOUT:
		//	return RTW_STATUS_TIMEDOUT;
		default:
			return _FAIL;
	}
}

inline u8* _rtw_zvmalloc(u32 sz)
{
	u8 	*pbuf;
	pbuf = vmalloc(sz);
	if (pbuf != NULL)
		memset(pbuf, 0, sz);
	return pbuf;	
}

inline void _rtw_vmfree(u8 *pbuf, u32 sz)
{
	vfree(pbuf);
}

u8* _rtw_malloc(u32 sz)
{
	u8 	*pbuf=NULL;

	pbuf = kmalloc(sz,in_interrupt() ? GFP_ATOMIC : GFP_KERNEL); 		

	return pbuf;	
}

u8* _rtw_zmalloc(u32 sz)
{
	u8 	*pbuf = _rtw_malloc(sz);

	if (pbuf != NULL) {
		memset(pbuf, 0, sz);
	}

	return pbuf;	
}

void	_rtw_mfree(u8 *pbuf, u32 sz)
{
	kfree(pbuf);
}

inline struct sk_buff *_rtw_skb_alloc(u32 sz)
{
	return __dev_alloc_skb(sz, in_interrupt() ? GFP_ATOMIC : GFP_KERNEL);
}

inline void _rtw_skb_free(struct sk_buff *skb)
{
	dev_kfree_skb_any(skb);
}

inline struct sk_buff *_rtw_skb_copy(const struct sk_buff *skb)
{
	return skb_copy(skb, in_interrupt() ? GFP_ATOMIC : GFP_KERNEL);
}

inline struct sk_buff *_rtw_skb_clone(struct sk_buff *skb)
{
	return skb_clone(skb, in_interrupt() ? GFP_ATOMIC : GFP_KERNEL);
}

inline int _rtw_netif_rx(_nic_hdl ndev, struct sk_buff *skb)
{
	skb->dev = ndev;
	return netif_rx(skb);
}

void _rtw_skb_queue_purge(struct sk_buff_head *list)
{
	struct sk_buff *skb;

	while ((skb = skb_dequeue(list)) != NULL)
		_rtw_skb_free(skb);
}

void _rtw_init_listhead(_list *list)
{

        INIT_LIST_HEAD(list);
}


/*
For the following list_xxx operations, 
caller must guarantee the atomic context.
Otherwise, there will be racing condition.
*/
u32	rtw_is_list_empty(_list *phead)
{

	if (list_empty(phead))
		return true;
	else
		return false;
}

void rtw_list_insert_head(_list *plist, _list *phead)
{
	list_add(plist, phead);
}

void rtw_list_insert_tail(_list *plist, _list *phead)
{

	list_add_tail(plist, phead);
}

void rtw_init_timer(_timer *ptimer, void *padapter, void *pfunc)
{
	_adapter *adapter = (_adapter *)padapter;	

	_init_timer(ptimer, adapter->pnetdev, pfunc, adapter);
}

/*

Caller must check if the list is empty before calling rtw_list_delete

*/


void	_rtw_init_queue(_queue	*pqueue)
{
	_rtw_init_listhead(&(pqueue->queue));

	spin_lock_init(&(pqueue->lock));
}

u32	  _rtw_queue_empty(_queue	*pqueue)
{
	return (rtw_is_list_empty(&(pqueue->queue)));
}


u32 rtw_end_of_queue_search(_list *head, _list *plist)
{
	if (head == plist)
		return true;
	else
		return false;
}

inline u32 rtw_systime_to_ms(u32 systime)
{
	return systime * 1000 / HZ;
}

inline s32 rtw_get_time_interval_ms(u32 start, u32 end)
{
	return rtw_systime_to_ms(end-start);
}

void rtw_sleep_schedulable(int ms)	
{
    u32 delta;
    
    delta = (ms * HZ)/1000;//(ms)
    if (delta == 0) {
        delta = 1;// 1 ms
    }
    set_current_state(TASK_INTERRUPTIBLE);
    if (schedule_timeout(delta) != 0) {
        return ;
    }
    return;
}

#define RTW_SUSPEND_LOCK_NAME "rtw_wifi"
#define RTW_SUSPEND_EXT_LOCK_NAME "rtw_wifi_ext"
#define RTW_SUSPEND_RX_LOCK_NAME "rtw_wifi_rx"
#define RTW_SUSPEND_TRAFFIC_LOCK_NAME "rtw_wifi_traffic"
#define RTW_SUSPEND_RESUME_LOCK_NAME "rtw_wifi_resume"
#define RTW_RESUME_SCAN_LOCK_NAME "rtw_wifi_scan"

inline void rtw_suspend_lock_init(void)
{
}

inline void rtw_suspend_lock_uninit(void)
{
}

inline void rtw_lock_suspend(void)
{
}

inline void rtw_unlock_suspend(void)
{
}

inline void rtw_resume_lock_suspend(void)
{
}

inline void rtw_resume_unlock_suspend(void)
{
}

inline void rtw_lock_suspend_timeout(u32 timeout_ms)
{
}

inline void rtw_lock_ext_suspend_timeout(u32 timeout_ms)
{
}

inline void rtw_lock_rx_suspend_timeout(u32 timeout_ms)
{
}


inline void rtw_lock_traffic_suspend_timeout(u32 timeout_ms)
{
}

inline void rtw_lock_resume_scan_timeout(u32 timeout_ms)
{
}

/*
* Open a file with the specific @param path, @param flag, @param mode
* @param fpp the pointer of struct file pointer to get struct file pointer while file opening is success
* @param path the path of the file to open
* @param flag file operation flags, please refer to linux document
* @param mode please refer to linux document
* @return Linux specific error code
*/
static int openFile(struct file **fpp, char *path, int flag, int mode) 
{ 
	struct file *fp; 
 
	fp=filp_open(path, flag, mode); 
	if(IS_ERR(fp)) {
		*fpp=NULL;
		return PTR_ERR(fp);
	}
	else {
		*fpp=fp; 
		return 0;
	}	
}

/*
* Close the file with the specific @param fp
* @param fp the pointer of struct file to close
* @return always 0
*/
static int closeFile(struct file *fp) 
{ 
	filp_close(fp,NULL);
	return 0; 
}

static int readFile(struct file *fp,char *buf,int len) 
{ 
	int rlen=0, sum=0;
	
	if (!fp->f_op || !fp->f_op->read) 
		return -EPERM;

	while(sum<len) {
		rlen=fp->f_op->read(fp,(char __force __user *)buf+sum,len-sum, &fp->f_pos);
		if(rlen>0)
			sum+=rlen;
		else if(0 != rlen)
			return rlen;
		else
			break;
	}
	
	return  sum;

}

static int writeFile(struct file *fp,char *buf,int len) 
{ 
	int wlen=0, sum=0;
	
	if (!fp->f_op || !fp->f_op->write) 
		return -EPERM; 

	while(sum<len) {
		wlen=fp->f_op->write(fp,(char __force __user *)buf+sum,len-sum, &fp->f_pos);
		if(wlen>0)
			sum+=wlen;
		else if(0 != wlen)
			return wlen;
		else
			break;
	}

	return sum;

}

/*
* Test if the specifi @param path is a file and readable
* @param path the path of the file to test
* @return Linux specific error code
*/
static int isFileReadable(char *path)
{ 
	struct file *fp;
	int ret = 0;
	mm_segment_t oldfs;
	char buf;
 
	fp=filp_open(path, O_RDONLY, 0); 
	if(IS_ERR(fp)) {
		ret = PTR_ERR(fp);
	}
	else {
		oldfs = get_fs(); set_fs(get_ds());
		
		if(1!=readFile(fp, &buf, 1))
			ret = PTR_ERR(fp);
		
		set_fs(oldfs);
		filp_close(fp,NULL);
	}	
	return ret;
}

/*
* Open the file with @param path and retrive the file content into memory starting from @param buf for @param sz at most
* @param path the path of the file to open and read
* @param buf the starting address of the buffer to store file content
* @param sz how many bytes to read at most
* @return the byte we've read, or Linux specific error code
*/
static int retriveFromFile(char *path, u8* buf, u32 sz)
{
	int ret =-1;
	mm_segment_t oldfs;
	struct file *fp;

	if(path && buf) {
		if( 0 == (ret=openFile(&fp,path, O_RDONLY, 0)) ){
			DBG_871X("%s openFile path:%s fp=%p\n",__FUNCTION__, path ,fp);

			oldfs = get_fs(); set_fs(get_ds());
			ret=readFile(fp, buf, sz);
			set_fs(oldfs);
			closeFile(fp);
			
			DBG_871X("%s readFile, ret:%d\n",__FUNCTION__, ret);
			
		} else {
			DBG_871X("%s openFile path:%s Fail, ret:%d\n",__FUNCTION__, path, ret);
		}
	} else {
		DBG_871X("%s NULL pointer\n",__FUNCTION__);
		ret =  -EINVAL;
	}
	return ret;
}

/*
* Open the file with @param path and wirte @param sz byte of data starting from @param buf into the file
* @param path the path of the file to open and write
* @param buf the starting address of the data to write into file
* @param sz how many bytes to write at most
* @return the byte we've written, or Linux specific error code
*/
static int storeToFile(char *path, u8* buf, u32 sz)
{
	int ret =0;
	mm_segment_t oldfs;
	struct file *fp;
	
	if(path && buf) {
		if( 0 == (ret=openFile(&fp, path, O_CREAT|O_WRONLY, 0666)) ) {
			DBG_871X("%s openFile path:%s fp=%p\n",__FUNCTION__, path ,fp);

			oldfs = get_fs(); set_fs(get_ds());
			ret=writeFile(fp, buf, sz);
			set_fs(oldfs);
			closeFile(fp);

			DBG_871X("%s writeFile, ret:%d\n",__FUNCTION__, ret);
			
		} else {
			DBG_871X("%s openFile path:%s Fail, ret:%d\n",__FUNCTION__, path, ret);
		}	
	} else {
		DBG_871X("%s NULL pointer\n",__FUNCTION__);
		ret =  -EINVAL;
	}
	return ret;
}

/*
* Test if the specifi @param path is a file and readable
* @param path the path of the file to test
* @return true or false
*/
int rtw_is_file_readable(char *path)
{
	if(isFileReadable(path) == 0)
		return true;
	else
		return false;
}

/*
* Open the file with @param path and retrive the file content into memory starting from @param buf for @param sz at most
* @param path the path of the file to open and read
* @param buf the starting address of the buffer to store file content
* @param sz how many bytes to read at most
* @return the byte we've read
*/
int rtw_retrive_from_file(char *path, u8* buf, u32 sz)
{
	int ret =retriveFromFile(path, buf, sz);
	return ret>=0?ret:0;
}

/*
* Open the file with @param path and wirte @param sz byte of data starting from @param buf into the file
* @param path the path of the file to open and write
* @param buf the starting address of the data to write into file
* @param sz how many bytes to write at most
* @return the byte we've written
*/
int rtw_store_to_file(char *path, u8* buf, u32 sz)
{
	int ret =storeToFile(path, buf, sz);
	return ret>=0?ret:0;
}

struct net_device *rtw_alloc_etherdev_with_old_priv(int sizeof_priv, void *old_priv)
{
	struct net_device *pnetdev;
	struct rtw_netdev_priv_indicator *pnpi;

	pnetdev = alloc_etherdev_mq(sizeof(struct rtw_netdev_priv_indicator), 4);
	if (!pnetdev)
		goto RETURN;
	
	pnpi = netdev_priv(pnetdev);
	pnpi->priv=old_priv;
	pnpi->sizeof_priv=sizeof_priv;

RETURN:
	return pnetdev;
}

struct net_device *rtw_alloc_etherdev(int sizeof_priv)
{
	struct net_device *pnetdev;
	struct rtw_netdev_priv_indicator *pnpi;

	pnetdev = alloc_etherdev_mq(sizeof(struct rtw_netdev_priv_indicator), 4);
	if (!pnetdev)
		goto RETURN;
	
	pnpi = netdev_priv(pnetdev);
	
	pnpi->priv = rtw_zvmalloc(sizeof_priv);
	if (!pnpi->priv) {
		free_netdev(pnetdev);
		pnetdev = NULL;
		goto RETURN;
	}
	
	pnpi->sizeof_priv=sizeof_priv;
RETURN:
	return pnetdev;
}

void rtw_free_netdev(struct net_device * netdev)
{
	struct rtw_netdev_priv_indicator *pnpi;
	
	if(!netdev)
		goto RETURN;
	
	pnpi = netdev_priv(netdev);

	if(!pnpi->priv)
		goto RETURN;

	rtw_vmfree(pnpi->priv, pnpi->sizeof_priv);
	free_netdev(netdev);

RETURN:
	return;
}

int rtw_change_ifname(_adapter *padapter, const char *ifname)
{
	struct net_device *pnetdev;
	struct net_device *cur_pnetdev;
	struct rereg_nd_name_data *rereg_priv;
	int ret;

	if(!padapter)
		goto error;

	cur_pnetdev = padapter->pnetdev;
	rereg_priv = &padapter->rereg_nd_name_priv;
	
	//free the old_pnetdev
	if(rereg_priv->old_pnetdev) {
		free_netdev(rereg_priv->old_pnetdev);
		rereg_priv->old_pnetdev = NULL;
	}

	if(!rtnl_is_locked())
		unregister_netdev(cur_pnetdev);
	else
		unregister_netdevice(cur_pnetdev);

	rereg_priv->old_pnetdev=cur_pnetdev;

	pnetdev = rtw_init_netdev(padapter);
	if (!pnetdev)  {
		ret = -1;
		goto error;
	}

	SET_NETDEV_DEV(pnetdev, dvobj_to_dev(adapter_to_dvobj(padapter)));

	rtw_init_netdev_name(pnetdev, ifname);

	memcpy(pnetdev->dev_addr, padapter->eeprompriv.mac_addr, ETH_ALEN);

	if(!rtnl_is_locked())
		ret = register_netdev(pnetdev);
	else
		ret = register_netdevice(pnetdev);

	if ( ret != 0) {
		RT_TRACE(_module_hci_intfs_c_,_drv_err_,("register_netdev() failed\n"));
		goto error;
	}

	return 0;

error:
	
	return -1;
	
}

u64 rtw_modular64(u64 x, u64 y)
{
	return do_div(x, y);
}

u64 rtw_division64(u64 x, u64 y)
{
	do_div(x, y);
	return x;
}

inline u32 rtw_random32(void)
{
	return prandom_u32();
}

void rtw_buf_free(u8 **buf, u32 *buf_len)
{
	u32 ori_len;

	if (!buf || !buf_len)
		return;

	ori_len = *buf_len;

	if (*buf) {
		u32 tmp_buf_len = *buf_len;
		*buf_len = 0;
		rtw_mfree(*buf, tmp_buf_len);
		*buf = NULL;
	}
}

void rtw_buf_update(u8 **buf, u32 *buf_len, u8 *src, u32 src_len)
{
	u32 ori_len = 0, dup_len = 0;
	u8 *ori = NULL;
	u8 *dup = NULL;

	if (!buf || !buf_len)
		return;

	if (!src || !src_len)
		goto keep_ori;

	/* duplicate src */
	dup = rtw_malloc(src_len);
	if (dup) {
		dup_len = src_len;
		memcpy(dup, src, dup_len);
	}

keep_ori:
	ori = *buf;
	ori_len = *buf_len;

	/* replace buf with dup */
	*buf_len = 0;
	*buf = dup;
	*buf_len = dup_len;

	/* free ori */
	if (ori && ori_len > 0)
		rtw_mfree(ori, ori_len);
}


/**
 * rtw_cbuf_full - test if cbuf is full
 * @cbuf: pointer of struct rtw_cbuf
 *
 * Returns: true if cbuf is full
 */
inline bool rtw_cbuf_full(struct rtw_cbuf *cbuf)
{
	return (cbuf->write == cbuf->read-1)? true : false;
}

/**
 * rtw_cbuf_empty - test if cbuf is empty
 * @cbuf: pointer of struct rtw_cbuf
 *
 * Returns: true if cbuf is empty
 */
inline bool rtw_cbuf_empty(struct rtw_cbuf *cbuf)
{
	return (cbuf->write == cbuf->read)? true : false;
}

/**
 * rtw_cbuf_push - push a pointer into cbuf
 * @cbuf: pointer of struct rtw_cbuf
 * @buf: pointer to push in
 *
 * Lock free operation, be careful of the use scheme
 * Returns: true push success
 */
bool rtw_cbuf_push(struct rtw_cbuf *cbuf, void *buf)
{
	if (rtw_cbuf_full(cbuf))
		return _FAIL;

	DBG_871X("%s on %u\n", __func__, cbuf->write);
	cbuf->bufs[cbuf->write] = buf;
	cbuf->write = (cbuf->write+1)%cbuf->size;

	return _SUCCESS;
}

/**
 * rtw_cbuf_pop - pop a pointer from cbuf
 * @cbuf: pointer of struct rtw_cbuf
 *
 * Lock free operation, be careful of the use scheme
 * Returns: pointer popped out
 */
void *rtw_cbuf_pop(struct rtw_cbuf *cbuf)
{
	void *buf;
	if (rtw_cbuf_empty(cbuf))
		return NULL;

        DBG_871X("%s on %u\n", __func__, cbuf->read);
	buf = cbuf->bufs[cbuf->read];
	cbuf->read = (cbuf->read+1)%cbuf->size;

	return buf;
}

/**
 * rtw_cbuf_alloc - allocte a rtw_cbuf with given size and do initialization
 * @size: size of pointer
 *
 * Returns: pointer of srtuct rtw_cbuf, NULL for allocation failure
 */
struct rtw_cbuf *rtw_cbuf_alloc(u32 size)
{
	struct rtw_cbuf *cbuf;

	cbuf = (struct rtw_cbuf *)rtw_malloc(sizeof(*cbuf) + sizeof(void*)*size);

	if (cbuf) {
		cbuf->write = cbuf->read = 0;
		cbuf->size = size;
	}

	return cbuf;
}

/**
 * rtw_cbuf_free - free the given rtw_cbuf
 * @cbuf: pointer of struct rtw_cbuf to free
 */
void rtw_cbuf_free(struct rtw_cbuf *cbuf)
{
	rtw_mfree((u8*)cbuf, sizeof(*cbuf) + sizeof(void*)*cbuf->size);
}

