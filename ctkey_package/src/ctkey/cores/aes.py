import os.path
from migen import *
from litex.soc.interconnect.csr import *
from litex.soc.integration.doc import AutoDoc, ModuleDoc

class aes(Module, AutoCSR, AutoDoc):
    def __init__(self):
        self.intro = ModuleDoc("""
        AES Core
        
        This core is a simple AES core that can be used to encrypt/decrypt data.
        
        The core has the following internal registers that can be accessed through the CSR bus
        by using the RST, CS, ADDR, WE, input and output registers.
        
        Address map for various registers
        ADDR_NAME0 - 0x00: Address for the first part of the core name
        ADDR_NAME1 - 0x01: Address for the second part of the core name
        ADDR_VERSION - 0x02: Address for the core version

        Control register and its bit positions
        ADDR_CTRL - 0x08: Address for the control register
        CTRL_INIT_BIT - 0: Bit position for the initialization control
        CTRL_NEXT_BIT - 1: Bit position for the next control
        CTRL_ENCDEC_BIT - 2: Bit position for encryption/decryption control (Only when reading)
        CTRL_KEYLEN_BIT - 3: Bit position for key length control (Only when reading)

        Status register and its bit positions
        ADDR_STATUS - 0x09: Address for the status register
        STATUS_READY_BIT - 0: Bit position indicating if the core is ready
        STATUS_VALID_BIT - 1: Bit position indicating if the output is valid

        Configuration register and its bit positions
        ADDR_CONFIG - 0x0a: Address for the configuration register
        CTRL_ENCDEC_BIT - 0: Bit position for encryption/decryption control (Only when writing)
        CTRL_KEYLEN_BIT - 1: Bit position for key length control (Only when writing)

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
        
        # CSR
        self.write_reg = CSRStorage(size=32, reset=0, name="input", description="Write register")
        self.read_reg = CSRStatus(size=32, reset=0, name="output", description="Read register")
        self.ctrl_reg = CSRStorage(fields=[
            CSRField("WE", size=1, reset=0, description="Write Enable. Set to 1 to write data set in WRITE_REG to the address set in ADDR."),
            CSRField("CS", size=1, reset=0, description="Chip Select. Set to 1 to enable core."),
            CSRField("RST", size=1, reset=1, description="Reset core. Active Low. Set to 0 to reset core."),
            CSRField("ADDR", size=8, description="Set address for read/write.")
        ], name="ctrl", description="Control register")
        
        self.specials += Instance("aes",
            i_clk=ClockSignal(),
            i_reset_n=self.ctrl_reg.fields.RST,
            i_cs=self.ctrl_reg.fields.CS,
            i_we=self.ctrl_reg.fields.WE,
            i_address=self.ctrl_reg.fields.ADDR,
            i_write_data=self.write_reg.storage,
            o_read_data=self.read_reg.status,
        )
        
        platform.add_source_dir(os.path.join(os.path.dirname(__file__), "aes/src/rtl"))
