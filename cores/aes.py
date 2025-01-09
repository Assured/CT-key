from migen import *
from litex.soc.interconnect.csr import *
from litex.soc.integration.doc import AutoDoc, ModuleDoc

import os.path

class aes(Module, AutoCSR, AutoDoc):
    def __init__(self, platform):
        self.intro = ModuleDoc("""
        AES Core
        
        This core is a simple AES core that can be used to encrypt/decrypt data.
        
        The core has the following internal registers that can be accessed through the CSR bus
        by using the ADDR, WE, WRITE_REG and READ_REG registers.
        
        Address map for various registers
        ADDR_NAME0 - 0x00: Address for the first part of the core name
        ADDR_NAME1 - 0x01: Address for the second part of the core name
        ADDR_VERSION - 0x02: Address for the core version

        Control register and its bit positions
        ADDR_CTRL - 0x08: Address for the control register
        CTRL_INIT_BIT - 0: Bit position for the initialization control
        CTRL_NEXT_BIT - 1: Bit position for the next control

        Status register and its bit positions
        ADDR_STATUS - 0x09: Address for the status register
        STATUS_READY_BIT - 0: Bit position indicating if the core is ready
        STATUS_VALID_BIT - 1: Bit position indicating if the output is valid

        Configuration register and its bit positions
        ADDR_CONFIG - 0x0a: Address for the configuration register
        CTRL_ENCDEC_BIT - 0: Bit position for encryption/decryption control
        CTRL_KEYLEN_BIT - 1: Bit position for key length control

        Key registers
        ADDR_KEY0 - 0x10: Address for the first key register
        ADDR_KEY7 - 0x17: Address for the last key register

        Block registers
        ADDR_BLOCK0 - 0x20: Address for the first block register
        ADDR_BLOCK3 - 0x23: Address for the last block register

        Result registers
        ADDR_RESULT0 - 0x30: Address for the first result register
        ADDR_RESULT3 - 0x33: Address for the last result register

        Core name and version constants
        CORE_NAME0 - 0x61657320: First part of the core name ("aes ")
        CORE_NAME1 - 0x20202020: Second part of the core name ("    ")
        CORE_VERSION - 0x302e3630: Core version ("0.60")
        """)
        
        self.platform = platform
        
        # CSR
        self.write_reg = CSRStorage(32, name="WRITE_REG", description="Write register")
        self.read_reg = CSRStatus(32, name="READ_REG", description="Read register")
        self.ctrl_reg = CSRStorage(fields=[
            CSRField("WE", size=1, reset=0, description="Write Enable. Set to 1 to write data set in WRITE_REG to the address set in ADDR. Will reset to 0 after write cycle."),
            CSRField("CS", size=1, reset=0, description="Chip Select. Set to 1 to enable core."),
            CSRField("RST", size=1, reset=1, pulse=True, description="Reset core. Will be automatically set to 1 after reset."),
            CSRField("ADDR", size=8, description="Set address for read/write.")
        ], name="CTRL_REG", description="Control register")
        
        # Signals
        cs = Signal()
        read_data = Signal(32)
        write_data = Signal(32)
        address = Signal(8)
        we = Signal()
        
        self.comb += [
            cs.eq(self.ctrl_reg.fields.CS),
            address.eq(self.ctrl_reg.fields.ADDR),
            we.eq(self.ctrl_reg.fields.WE),
            write_data.eq(self.write_reg.storage),
            self.read_reg.status.eq(read_data)
        ]
        
        self.specials += Instance("aes",
            i_clk=ClockSignal(),
            i_reset_n=ResetSignal() | self.ctrl_reg.fields.RST,
            i_cs=cs,
            i_we=we,
            i_address=address,
            i_write_data=write_data,
            o_read_data=read_data
        )
        
        self.platform.add_source_dir(os.path.join(os.path.dirname(__file__), "aes/src/rtl"))