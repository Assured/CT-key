from migen import *
from litex.soc.interconnect.csr import *
from litex.soc.integration.doc import AutoDoc, ModuleDoc

import os.path

class timer(Module, AutoCSR, AutoDoc):
    def __init__(self, platform):
        self.intro = ModuleDoc("""Timer Core""")
        self.platform = platform

        self.write_reg = CSRStorage(32, name="WRITE_REG", description="Timer write register")
        self.read_reg = CSRStatus(32, name="READ_REG", description="Timer read register")
        self.ctrl_reg = CSRStorage(fields=[
            CSRField("WE", size=1, reset=0, description="When set to 1, the timer is in write mode. When set to 0, the timer is in read mode."),
            CSRField("CS", size=1, reset=0, description="When set to 1, the timer is enabled"),
            CSRField("RST", size=1, reset=1, pulse=True, description="When set to 0, reset the timer. Will be automatically set to 1 after reset."),
            CSRField("ADDR", size=8, description="Timer register address for read/write.")
        ], name="CTRL_REG", description="Timer control register")
        self.status_reg = CSRStatus(fields=[
            CSRField("READY", size=1, access=CSRAccess.ReadOnly, description="Timer ready is set to 1 when the timer is enabled and ready to read/write.")
        ], name="STATUS_REG", description="Timer status register")
    
        # Signals
        ready = Signal()
        cs = Signal()
        read_data = Signal(32)
        write_data = Signal(32)
        address = Signal(8)
        we = Signal()

        self.comb += [
            self.status_reg.fields.READY.eq(ready),
            cs.eq(self.ctrl_reg.fields.CS),
            address.eq(self.ctrl_reg.fields.ADDR),
            we.eq(self.ctrl_reg.fields.WE),
            write_data.eq(self.write_reg.storage),
            self.read_reg.status.eq(read_data)
        ]

        self.specials += Instance("timer",
            i_clk=ClockSignal(),
            i_reset_n=ResetSignal() | self.ctrl_reg.fields.RST,
            i_cs=cs,
            i_we=we,
            i_address=address,
            i_write_data=write_data,
            o_read_data=read_data,
            o_ready=ready,
        )
        
        self.platform.add_source_dir(os.path.join(os.path.dirname(__file__), "timer"))
