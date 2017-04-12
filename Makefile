install:
	@mkdir -p /lib/firmware/rtlwifi/
	@cp -n rtl8723bs_nic.bin /lib/firmware/rtlwifi/rtl8723bs_nic.bin
	@cp -n rtl8723bs_wowlan.bin /lib/firmware/rtlwifi/rtl8723bs_wowlan.bin
