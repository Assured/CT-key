#!/usr/bin/env python3

from litex_boards.targets.radiona_ulx3s import BaseSoC
from litex_boards.platforms import radiona_ulx3s
from litex.soc.integration.builder import *
from cores import timer

class SoCUlx3s(BaseSoC):
    def __init__(self, **kwargs):
        BaseSoC.__init__(self, **kwargs)

        # Timer ------------------------------------------------------------------------------------
        self.timer = timer.timer(self.platform)
    
    # RGB Led ----------------------------------------------------------------------------------
    
    def add_rgb_led(self):
        rgb_led_pads = self.platform.request("rgb_led", 0)
        for n in "rgb":
            self.add_module(name=f"rgb_led_{n}0", module=PWM(getattr(rgb_led_pads, n)))

    # Switches ---------------------------------------------------------------------------------

    def add_switches(self):
        self.switches = GPIOIn(Cat(self.platform.request_all("user_sw")), with_irq=True)
        self.irq.add("switches")

    # SPI --------------------------------------------------------------------------------------

    def add_spi(self, data_width, clk_freq):
        spi_pads = self.platform.request("spi")
        self.spi = SPIMaster(spi_pads, data_width, self.clk_freq, clk_freq)

    # I2C --------------------------------------------------------------------------------------

    def add_i2c(self):
        self.i2c0 = I2CMaster(self.platform.request("i2c", 0))

    # Ethernet configuration -------------------------------------------------------------------

    def configure_ethernet(self, remote_ip):
        remote_ip = remote_ip.split(".")
        try: # FIXME: Improve.
            self.constants.pop("REMOTEIP1")
            self.constants.pop("REMOTEIP2")
            self.constants.pop("REMOTEIP3")
            self.constants.pop("REMOTEIP4")
        except:
            pass
        self.add_constant("REMOTEIP1", int(remote_ip[0]))
        self.add_constant("REMOTEIP2", int(remote_ip[1]))
        self.add_constant("REMOTEIP3", int(remote_ip[2]))
        self.add_constant("REMOTEIP4", int(remote_ip[3]))

    # DTS generation ---------------------------------------------------------------------------

    def generate_dts(self, board_name):
        json_src = os.path.join("build", board_name, "csr.json")
        dts = os.path.join("build", board_name, "{}.dts".format(board_name))

        with open(json_src) as json_file, open(dts, "w") as dts_file:
            dts_content = generate_dts(json.load(json_file), polling=False)
            dts_file.write(dts_content)

    # DTS compilation --------------------------------------------------------------------------

    def compile_dts(self, board_name, symbols=False):
        dts = os.path.join("build", board_name, "{}.dts".format(board_name))
        dtb = os.path.join("build", board_name, "{}.dtb".format(board_name))
        subprocess.check_call(
            "dtc {} -O dtb -o {} {}".format("-@" if symbols else "", dtb, dts), shell=True)

    # DTB combination --------------------------------------------------------------------------

    def combine_dtb(self, board_name, overlays=""):
        dtb_in = os.path.join("build", board_name, "{}.dtb".format(board_name))
        dtb_out = os.path.join("images", "rv32.dtb")
        if overlays == "":
            shutil.copyfile(dtb_in, dtb_out)
        else:
            subprocess.check_call(
                "fdtoverlay -i {} -o {} {}".format(dtb_in, dtb_out, overlays), shell=True)

    # Documentation generation -----------------------------------------------------------------
    def generate_doc(self, board_name):
        from litex.soc.doc import generate_docs
        doc_dir = os.path.join("build", board_name, "doc")
        generate_docs(self, doc_dir)
        os.system("sphinx-build -M html {}/ {}/_build".format(doc_dir, doc_dir))
        
def main():
    from litex.build.parser import LiteXArgumentParser
    
    parser = LiteXArgumentParser(platform=radiona_ulx3s.Platform, description="LiteX SoC on ULX3S")
    parser.add_target_argument("--device",          default="LFE5U-45F",      help="FPGA device (LFE5U-12F, LFE5U-25F, LFE5U-45F or LFE5U-85F).")
    parser.add_target_argument("--revision",        default="2.0",            help="Board revision (2.0 or 1.7).")
    parser.add_target_argument("--sys-clk-freq",    default=50e6, type=float, help="System clock frequency.")
    parser.add_target_argument("--sdram-module",    default="MT48LC16M16",    help="SDRAM module (MT48LC16M16, AS4C32M16 or AS4C16M16).")
    parser.add_target_argument("--with-spi-flash",  action="store_true",      help="Enable SPI Flash (MMAPed).")
    sdopts = parser.target_group.add_mutually_exclusive_group()
    sdopts.add_argument("--with-spi-sdcard",   action="store_true", help="Enable SPI-mode SDCard support.")
    sdopts.add_argument("--with-sdcard",       action="store_true", help="Enable SDCard support.")
    parser.add_target_argument("--with-oled",  action="store_true", help="Enable SDD1331 OLED support.")
    parser.add_target_argument("--sdram-rate", default="1:1",       help="SDRAM Rate (1:1 Full Rate or 1:2 Half Rate).")
    viopts = parser.target_group.add_mutually_exclusive_group()
    viopts.add_argument("--with-video-terminal",    action="store_true", help="Enable Video Terminal (HDMI).")
    viopts.add_argument("--with-video-framebuffer", action="store_true", help="Enable Video Framebuffer (HDMI).")
    args = parser.parse_args()

    soc = SoCUlx3s(
        device                 = args.device,
        revision               = args.revision,
        toolchain              = args.toolchain,
        sys_clk_freq           = args.sys_clk_freq,
        sdram_module_cls       = args.sdram_module,
        sdram_rate             = args.sdram_rate,
        with_video_terminal    = args.with_video_terminal,
        with_video_framebuffer = args.with_video_framebuffer,
        with_spi_flash         = args.with_spi_flash,
        **parser.soc_argdict)
    if args.with_spi_sdcard:
        soc.add_spi_sdcard()
    if args.with_sdcard:
        soc.add_sdcard()
    if args.with_oled:
        soc.add_oled()

    builder = Builder(soc, **parser.builder_argdict)
    if args.build:
        builder.build(**parser.toolchain_argdict)

    if args.load:
        prog = soc.platform.create_programmer()
        prog.load_bitstream(builder.get_bitstream_filename(mode="sram", ext=".svf")) # FIXME

if __name__ == "__main__":
    main()