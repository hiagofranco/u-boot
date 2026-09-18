# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2026, Hiago De Franco <hfranco@baylibre.com>
#
# Entry-type module for the Sophgo CV18xx BootROM FIP image
#

import binascii
import struct

from binman.etype.section import Entry_section
from u_boot_pylib import tools

FIP_ALIGN = 0x200
PARAM1_SIZE = 0x1000

# jal zero, +32: jump over the BL2 header expected by the BootROM
BL2_HEADER = struct.pack('<I', 0x0200006f) + tools.get_bytes(0, 28)

# The subnodes follow param1 and the BL2 header
SPL_OFFSET = PARAM1_SIZE + len(BL2_HEADER)

# Empty register list, only the "scan start" terminator
CHIP_CONF = struct.pack('<II', 0xffffffa0, 0xffffffff)


def fip_cksum(data):
    """CRC-16/XMODEM followed by the 0xcafe marker"""
    return struct.pack('<HH', binascii.crc_hqx(data, 0), 0xcafe)


class Entry_sophgo_cv18xx_fip(Entry_section):
    """Sophgo CV18xx FIP image

    This creates the FIP image loaded by the BootROM of the Sophgo CV180x and
    CV181x SoCs, normally named fip.bin on the boot media. The subnodes,
    typically just u-boot-spl, form the BL2 image, which the BootROM loads
    into SRAM and runs::

        sophgo-cv18xx-fip {
            u-boot-spl {
            };
        };

    The image contains:

    - param1: 4KiB header with the BL2 size and checksums, read by the BootROM
    - BL2: a 32-byte header whose first instruction jumps over it, followed by
      the subnodes, aligned to 512 bytes

    Unlike the vendor fiptool, no BLCP, param2, monitor or second stage loader
    is packed, as the BootROM does not use them: SPL loads the next stages
    itself, e.g. from a FIT placed after this entry.
    """

    def _PackEntries(self):
        """Pack the subnodes after param1 and the BL2 header"""
        offset = SPL_OFFSET
        for entry in self.GetEntries().values():
            offset = entry.Pack(offset)
        return offset

    def BuildSectionData(self, required):
        data = super().BuildSectionData(required)
        if data is None:
            return None

        # Once packed, the subnodes are preceded by the space reserved for
        # param1 and the BL2 header, which is filled in below
        if all(entry.offset is not None for entry in self.GetEntries().values()):
            if any(data[:SPL_OFFSET]):
                self.Raise('Subnodes must be placed at %#x or later' %
                           SPL_OFFSET)
            data = data[SPL_OFFSET:]

        bl2 = BL2_HEADER + data
        bl2 += tools.get_bytes(0, tools.align(len(bl2), FIP_ALIGN) - len(bl2))

        param1 = bytearray(PARAM1_SIZE)
        param1[0x00:0x08] = b'CVBL01\n\0'
        param1[0x90:0xb4] = tools.get_bytes(0xff, 36)           # NOR_INFO
        struct.pack_into('<I', param1, 0xbc, len(CHIP_CONF))    # CHIP_CONF_SIZE
        param1[0xc0:0xc4] = fip_cksum(b'')                      # BLCP_IMG_CKSUM
        param1[0xd4:0xd8] = fip_cksum(bl2)                      # BL2_IMG_CKSUM
        struct.pack_into('<I', param1, 0xd8, len(bl2))          # BL2_IMG_SIZE
        param1[0xe8:0xe8 + len(CHIP_CONF)] = CHIP_CONF
        # PARAM_CKSUM covers NAND_INFO up to the signatures
        param1[0x0c:0x10] = fip_cksum(param1[0x10:0x800])

        return bytes(param1) + bl2
