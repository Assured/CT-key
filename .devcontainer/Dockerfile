FROM amd64/alpine:3.20.3

# Set the working directory
WORKDIR /tools

# Install Python 3.6+ and verilator
RUN apk add --no-cache python3 py3-pip verilator git py3-virtualenv make json-c-dev libevent-dev bash

# Install yosys
RUN wget https://github.com/YosysHQ/oss-cad-suite-build/releases/download/2025-01-08/oss-cad-suite-linux-x64-20250108.tgz && \
    tar -xvf oss-cad-suite-linux-x64-20250108.tgz && \
    rm oss-cad-suite-linux-x64-20250108.tgz
ENV PATH="/tools/oss-cad-suite/bin:$PATH"

# Create virtualenv and activate it
ENV VIRTUAL_ENV=/opt/venv
RUN python3 -m venv $VIRTUAL_ENV
ENV PATH="$VIRTUAL_ENV/bin:$PATH"

# Install litex
RUN wget https://raw.githubusercontent.com/enjoy-digital/litex/master/litex_setup.py && \
    python3 litex_setup.py --init --install --config=standard

# Install riscv-gnu-toolchain
RUN pip3 install meson ninja
RUN python3 litex_setup.py --gcc=riscv

# Add user
RUN adduser -D -u 1000 litex
USER litex

# Set the working directory
WORKDIR /home/litex

# Set the entrypoint
ENTRYPOINT ["/bin/bash"]

