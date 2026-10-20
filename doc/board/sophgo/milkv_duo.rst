.. SPDX-License-Identifier: GPL-2.0+

Milk-V Duo
==========

CV1800B RISC-V SoC
------------------
The CV1800B is a high-performance, low-power 1+1 64-bit RISC-V SoC from Sophgo.

Mainline support
----------------
Support for the following drivers is already enabled:

1. ns16550 UART Driver.
2. Synopsys DesignWare MSHC Driver.
3. Synopsys DesignWare Ethernet Controller.
4. SPL with DDR2 initialization.

Building
~~~~~~~~
1. Add the RISC-V toolchain to your PATH.
2. Build mainline OpenSBI for the generic platform:

.. code-block:: console

   cd <OpenSBI-dir>
   PLATFORM=generic CROSS_COMPILE=<riscv64 toolchain prefix> make

This will generate build/platform/generic/firmware/fw_dynamic.bin.

3. Build U-Boot, pointing OPENSBI to the fw_dynamic.bin file:

.. code-block:: console

   export CROSS_COMPILE=<riscv64 toolchain prefix>
   cd <U-Boot-dir>
   make milkv_duo_defconfig
   OPENSBI=<OpenSBI-dir>/build/platform/generic/firmware/fw_dynamic.bin make

This will generate fip.bin, the image the BootROM loads. Binman builds it from
the SPL, with the 32-byte BL2 header the BootROM expects, followed at offset
0x40000 by a FIT image containing OpenSBI, U-Boot and the device tree. The
vendor fiptool is not needed.

Booting
~~~~~~~
The BootROM loads the SPL from the fip.bin file in the FAT partition of the SD
card. The SPL initializes the DDR and the PLLs, then loads the FIT image from
offset 0x40000 of fip.bin through the BootROM API and jumps to OpenSBI, which
starts U-Boot in S-mode.

Copy fip.bin to the FAT partition of the SD card, insert the card into the
board and power it on.

Sample boot log from Milk-V Duo board
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
.. code-block:: none

  U-Boot SPL 2026.10-00011-gf0ec1f67053d (Oct 20 2026 - 15:08:30 -0300)
  Trying to boot from ROMAPI
  SD/0x40000/0x200/0x200/0.SD/0x40000/0xca000/0xc9ee9/0.
  OpenSBI v1.9
  ...

  U-Boot 2026.10-00011-gf0ec1f67053d (Oct 20 2026 - 15:08:30 -0300)milkv_duo

  DRAM:  63.3 MiB
  Core:  23 devices, 15 uclasses, devicetree: separate
  MMC:   mmc@4310000: 0
  Loading Environment from nowhere... OK
  In:    serial@4140000
  Out:   serial@4140000
  Err:   serial@4140000
  Net:
  Warning: ethernet@4070000 (eth0) using random MAC address - be:2a:c8:b9:53:10
  eth0: ethernet@4070000
  Hit any key to stop autoboot: 0
  milkv_duo#
