.. SPDX-License-Identifier: GPL-2.0+

Milk-V Duo 256M
===============

SG2002 RISC-V SoC
------------------
The SG2002 is a high-performance, low-power 64-bit RISC-V/ARM SoC from Sophgo.

Mainline support
----------------
The support for following drivers are already enabled:

1. ns16550 UART Driver.
2. Synopsys Designware MSHC Driver.
3. Synopsys Designware Ethernet Controller.
4. SPL with DDR3 initialization.

Building
~~~~~~~~
1. Add the RISC-V toolchain to your PATH.
2. Build mainline OpenSBI (1.5 or newer) for the generic platform:

.. code-block:: console

   cd <OpenSBI-dir>
   make PLATFORM=generic CROSS_COMPILE=<riscv64 toolchain prefix>

This will generate build/platform/generic/firmware/fw_dynamic.bin.

3. Build U-Boot, pointing OPENSBI to the fw_dynamic.bin file:

.. code-block:: console

   export CROSS_COMPILE=<riscv64 toolchain prefix>
   cd <U-Boot-dir>
   make milkv_duo_256m_defconfig
   make OPENSBI=<OpenSBI-dir>/build/platform/generic/firmware/fw_dynamic.bin

This will generate fip.bin, the image the BootROM loads. Binman builds it from
the SPL, with the 32-byte BL2 header the BootROM expects, followed at offset
0x40000 by u-boot.itb, a FIT image containing OpenSBI, U-Boot and the device
tree. The vendor fiptool is not needed.

Booting
~~~~~~~
The BootROM loads the SPL from the fip.bin file in the FAT partition of the SD
card. The SPL initializes the DDR and the PLLs, then loads u-boot.itb from
offset 0x40000 of fip.bin through the BootROM API and jumps to OpenSBI, which
starts U-Boot in S-mode.

Copy fip.bin to the FAT partition of the SD card, insert the card into the
board and power it on.

Sample boot log from Milk-V 256M Duo board
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
.. code-block:: none

  U-Boot SPL 2026.10-rc4-00001-g6b34cfc72b0f (Sep 16 2026 - 17:08:19 -0300)
  Trying to boot from ROMAPI
  SD/0x40000/0x200/0x200/0.SD/0x40000/0xd0a00/0xd09b1/0.
  OpenSBI v1.9
  ...

  U-Boot 2026.10-rc4-00001-g6b34cfc72b0f (Sep 16 2026 - 17:08:19 -0300)milkv_duo_256m

  DRAM:  256 MiB
  Core:  20 devices, 13 uclasses, devicetree: separate
  MMC:   mmc@4310000: 0
  Loading Environment from nowhere... OK
  In:    serial@4140000
  Out:   serial@4140000
  Err:   serial@4140000
  Net:
  Warning: ethernet@4070000 (eth0) using random MAC address - ba:0f:14:06:d2:e3
  eth0: ethernet@4070000
  Hit any key to stop autoboot: 0
  milkv_duo_256m#
