#!/usr/bin/env python3

#
# This file is part of Linux-on-LiteX-VexRiscv
#
# Copyright (c) 2019-2024, Linux-on-LiteX-VexRiscv Developers
# SPDX-License-Identifier: BSD-2-Clause

import json
import argparse

from migen import *

from litex.build.generic_platform import *
from litex.tools.litex_sim    import SimSoC
from litex.build.sim              import SimPlatform
from litex.build.sim.config       import SimConfig
from litex.soc.integration.common   import *
from litex.soc.integration.soc_core import *
from litex.soc.integration.builder  import *
from litex.soc.integration.soc      import *
from litex.soc.cores.bitbang import *
from litex.soc.cores.gpio    import GPIOTristate
from litex.soc.cores.cpu     import CPUS
from litex.soc.cores.video   import VideoGenericPHY

from litedram           import modules as litedram_modules
from litedram.modules   import parse_spd_hexdump
from litedram.phy.model import sdram_module_nphases, get_sdram_phy_settings
from litedram.phy.model import SDRAMPHYModel

from liteeth.common             import *
from liteeth.phy.gmii           import LiteEthPHYGMII
from liteeth.phy.xgmii          import LiteEthPHYXGMII
from liteeth.phy.model          import LiteEthPHYModel
from liteeth.mac                import LiteEthMAC
from liteeth.core.arp           import LiteEthARP
from liteeth.core.ip            import LiteEthIP
from liteeth.core.udp           import LiteEthUDP
from liteeth.core.icmp          import LiteEthICMP
from liteeth.core               import LiteEthUDPIPCore
from liteeth.frontend.etherbone import LiteEthEtherbone

from litescope import LiteScopeAnalyzer

from cores import timer

# SoCLinux -----------------------------------------------------------------------------------------

class SimUlx3s(SimSoC):
    def __init__(self, **kwargs):
        SimSoC.__init__(self, **kwargs)

        # Timer ------------------------------------------------------------------------------------
        self.timer = timer.timer(self.platform)
        
# Build --------------------------------------------------------------------------------------------
def sim_args(parser):
    # ROM / RAM.
    parser.add_argument("--rom-init",             default=None,            help="ROM init file (.bin or .json).")
    parser.add_argument("--ram-init",             default=None,            help="RAM init file (.bin or .json).")

    # DRAM.
    parser.add_argument("--with-sdram",           action="store_true",     help="Enable SDRAM support.")
    parser.add_argument("--with-sdram-bist",      action="store_true",     help="Enable SDRAM BIST Generator/Checker modules.")
    parser.add_argument("--sdram-module",         default="MT48LC16M16",   help="Select SDRAM chip.")
    parser.add_argument("--sdram-data-width",     default=32,              help="Set SDRAM chip data width.")
    parser.add_argument("--sdram-init",           default=None,            help="SDRAM init file (.bin or .json).")
    parser.add_argument("--sdram-from-spd-dump",  default=None,            help="Generate SDRAM module based on data from SPD EEPROM dump.")
    parser.add_argument("--sdram-verbosity",      default=0,               help="Set SDRAM checker verbosity.")

    # Ethernet /Etherbone.
    parser.add_argument("--with-ethernet",        action="store_true",     help="Enable Ethernet support.")
    parser.add_argument("--ethernet-phy-model",   default="sim",           help="Ethernet PHY to simulate (sim, xgmii or gmii).")
    parser.add_argument("--with-etherbone",       action="store_true",     help="Enable Etherbone support.")
    parser.add_argument("--local-ip",             default="192.168.1.50",  help="Local IP address of SoC.")
    parser.add_argument("--remote-ip",            default="192.168.1.100", help="Remote IP address of TFTP server.")

    # SDCard.
    parser.add_argument("--with-sdcard",          action="store_true",     help="Enable SDCard support.")

    # SPIFlash.
    parser.add_argument("--with-spi-flash",       action="store_true",     help="Enable SPI Flash (MMAPed).")
    parser.add_argument("--spi_flash-init",       default=None,            help="SPI Flash init file.")

    # I2C.
    parser.add_argument("--with-i2c",             action="store_true",     help="Enable I2C support.")

    # JTAG
    parser.add_argument("--with-jtagremote",      action="store_true", help="Enable jtagremote support")

    # GPIO.
    parser.add_argument("--with-gpio",            action="store_true",     help="Enable Tristate GPIO (32 pins).")

    # Analyzer.
    parser.add_argument("--with-analyzer",        action="store_true",     help="Enable Analyzer support.")

    # Video.
    parser.add_argument("--with-video-framebuffer", action="store_true",   help="Enable Video Framebuffer.")
    parser.add_argument("--with-video-terminal",    action="store_true",   help="Enable Video Terminal.")
    parser.add_argument("--with-video-colorbars",   action="store_true",   help="Enable Video test pattern.")
    parser.add_argument("--video-vsync",            action="store_true",   help="Only render on frame vsync.")

    # Debug/Waveform.
    parser.add_argument("--sim-debug",            action="store_true",     help="Add simulation debugging modules.")
    parser.add_argument("--gtkwave-savefile",     action="store_true",     help="Generate GTKWave savefile.")
    parser.add_argument("--non-interactive",      action="store_true",     help="Run simulation without user input.")
    
def main():
    from litex.build.parser import LiteXArgumentParser
    parser = LiteXArgumentParser(description="LiteX SoC Simulation utility")
    parser.set_platform(SimPlatform)
    sim_args(parser)
    args = parser.parse_args()

    soc_kwargs = soc_core_argdict(args)

    sys_clk_freq = int(1e6)
    sim_config   = SimConfig()
    sim_config.add_clocker("sys_clk", freq_hz=sys_clk_freq)

    # Configuration --------------------------------------------------------------------------------

    # UART.
    if soc_kwargs["uart_name"] == "serial":
        soc_kwargs["uart_name"] = "sim"
        sim_config.add_module("serial2console", "serial")

    # Create config SoC that will be used to prepare/configure real one.
    conf_soc = SimUlx3s(**soc_kwargs)

    # ROM.
    if args.rom_init:
        soc_kwargs["integrated_rom_init"] = get_mem_data(args.rom_init,
            data_width = conf_soc.bus.data_width,
            endianness = conf_soc.cpu.endianness
        )

    # RAM / SDRAM.
    ram_boot_address = None
    soc_kwargs["integrated_main_ram_size"] = args.integrated_main_ram_size
    if args.integrated_main_ram_size:
        if args.ram_init is not None:
            soc_kwargs["integrated_main_ram_init"] = get_mem_data(args.ram_init,
                data_width = conf_soc.bus.data_width,
                endianness = conf_soc.cpu.endianness,
                offset     = conf_soc.mem_map["main_ram"]
            )
            ram_boot_address = get_boot_address(args.ram_init)
    elif args.with_sdram:
        assert args.ram_init is None
        soc_kwargs["sdram_module"]     = args.sdram_module
        soc_kwargs["sdram_data_width"] = int(args.sdram_data_width)
        soc_kwargs["sdram_verbosity"]  = int(args.sdram_verbosity)
        if args.sdram_from_spd_dump:
            soc_kwargs["sdram_spd_data"] = parse_spd_hexdump(args.sdram_from_spd_dump)
        if args.sdram_init is not None:
            soc_kwargs["sdram_init"] = get_mem_data(args.sdram_init,
                data_width = conf_soc.bus.data_width,
                endianness = conf_soc.cpu.endianness,
                offset     = conf_soc.mem_map["main_ram"]
            )
            ram_boot_address = get_boot_address(args.sdram_init)

    # Ethernet.
    if args.with_ethernet or args.with_etherbone:
        if args.ethernet_phy_model == "sim":
            sim_config.add_module("ethernet", "eth", args={"interface": "tap0", "ip": args.remote_ip})
        elif args.ethernet_phy_model == "xgmii":
            sim_config.add_module("xgmii_ethernet", "xgmii_eth", args={"interface": "tap0", "ip": args.remote_ip})
        elif args.ethernet_phy_model == "gmii":
            sim_config.add_module("gmii_ethernet", "gmii_eth", args={"interface": "tap0", "ip": args.remote_ip})
        else:
            raise ValueError("Unknown Ethernet PHY model: " + args.ethernet_phy_model)

    # I2C.
    if args.with_i2c:
        sim_config.add_module("spdeeprom", "i2c")

    # JTAG
    if args.with_jtagremote:
        sim_config.add_module("jtagremote", "jtag", args={'port': 44853})

    # Video.
    if args.with_video_framebuffer or args.with_video_terminal or args.with_video_colorbars:
        sim_config.add_module("video", "vga", args={"render_on_vsync": args.video_vsync})

    # SoC ------------------------------------------------------------------------------------------
    soc = SimUlx3s(
        with_sdram             = args.with_sdram,
        with_sdram_bist        = args.with_sdram_bist,
        with_ethernet          = args.with_ethernet,
        ethernet_phy_model     = args.ethernet_phy_model,
        with_etherbone         = args.with_etherbone,
        with_analyzer          = args.with_analyzer,
        with_i2c               = args.with_i2c,
        with_jtag              = args.with_jtagremote,
        with_sdcard            = args.with_sdcard,
        with_spi_flash         = args.with_spi_flash,
        with_gpio              = args.with_gpio,
        with_video_framebuffer = args.with_video_framebuffer,
        with_video_terminal    = args.with_video_terminal,
        with_video_colorbars   = args.with_video_colorbars,
        sim_debug              = args.sim_debug,
        trace_reset_on         = int(float(args.trace_start)) > 0 or int(float(args.trace_end)) > 0,
        spi_flash_init         = None if args.spi_flash_init is None else get_mem_data(args.spi_flash_init, endianness="big"),
        **soc_kwargs)
    if ram_boot_address is not None:
        if ram_boot_address == 0:
            ram_boot_address = conf_soc.mem_map["main_ram"]
        soc.add_constant("ROM_BOOT_ADDRESS", ram_boot_address)
    if args.with_ethernet:
        for i in range(4):
            soc.add_constant(f"LOCALIP{i+1}", int(args.local_ip.split(".")[i]))
        for i in range(4):
            soc.add_constant(f"REMOTEIP{i+1}", int(args.remote_ip.split(".")[i]))

    # Build/Run ------------------------------------------------------------------------------------
    def pre_run_callback(vns):
        if args.trace:
            generate_gtkw_savefile(builder, vns, args.trace_fst)

    builder = Builder(soc, **parser.builder_argdict)
    builder.build(
        sim_config       = sim_config,
        interactive      = not args.non_interactive,
        video            = args.with_video_framebuffer or args.with_video_terminal or args.with_video_colorbars,
        pre_run_callback = pre_run_callback,
        **parser.toolchain_argdict,
    )

if __name__ == "__main__":
    main()
