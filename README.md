# rtl8723bs
Realtek SDIO Wi-Fi driver

## History

This repository originally contained a vendor driver provided by Realtek:
`rtl8723BS_WiFi_linux_v4.3.5.5_12290.20140916_BTCOEX20140507-4E40`

It was then tidied up and had huge chunks of unused code removed by a number
of contributors (see the git history for details), and eventually merged into
the kernel staging tree in time for kernel 4.12-rc1.

## Getting the code

This repository contains the history of the driver cleanup, which is not
available in the upstream-merged driver. Compiling or running the
driver on older kernels is not supported, please ask your distribution to
provide updated kernel packages.

For support running the upstreamed staging driver, please refer to the
`NETWORKING [WIRELESS]` section of
[MAINTAINERS](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/tree/MAINTAINERS) file to find where to send requests.

To provide patches, see the [driver TODO](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/tree/drivers/staging/rtl8723bs/TODO).

## Getting the firmware

The various firmwares were added to the [linux-firmware](https://git.kernel.org/pub/scm/linux/kernel/git/firmware/linux-firmware.git/) repository [in February 2017](https://git.kernel.org/pub/scm/linux/kernel/git/firmware/linux-firmware.git/commit/?id=28b9f21e555c6c0d28e954c76ec3e22ba487b1c7)

## Tested list (old)

Tested on:
- Onda v975w
- Teclast 3G
- HP Stream 7
- Dell Venue 8 3000
- WinBook TW100 and TW700
- Acer Aspire Switch 10E
- Lenovo Miix 3-830 and 3-1030
- Medion Akoya S2217
- Chuwi Hi12
- Intel Compute Stick STCK1A32WFC
- Lark Ultimate 7i WIN

And many others.
