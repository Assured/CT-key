# CT-key

## Hardware

The design is being developed for ECP5 FPGA and been tested on ulx3s and orangecrab boards. Still in development phase on what hardware to use. The CPU architecture that is going to be used is RISC-V for the softcore.

The hardware is built with the Litex framework where various peripherals are used.

## Software, Toolchain and Libraries

### List of software and versions

| Name | Version | Location | Arguments | Notes |
| --- | --- | --- | --- | --- |
| Litex | latest | [Link](https://raw.githubusercontent.com/enjoy-digital/litex/master/litex_setup.py) | --init --install --user | Requires a python virtualenv to be created first and Python 3.12. |
| riscv64-unknown-elf-toolchain | 10.2.0-2020.12.8 | [Link](https://static.dev.sifive.com/dev-tools/freedom-tools/v2020.12/riscv64-unknown-elf-toolchain-10.2.0-2020.12.8-x86_64-linux-ubuntu14.tar.gz) | None | Just unpack to location of your choice and add the directory to your PATH. |
| Yosys | 20250108 | [Link](https://github.com/YosysHQ/oss-cad-suite-build/releases/download/2025-01-08/oss-cad-suite-linux-x64-20250108.tgz) | None | Just unpack to location of your choice and add the directory to your PATH. |
| ctkey-package | latest | [Link](ctkey_package/) | source <path to litex venv>/bin/activate; python -m build; pip install dist/*.whl | None | Installs the cores and targets needed by Litex for ctkey. |
| buildroot | 2024.02.10 (Latest LTS) | [Link](https://github.com/buildroot/buildroot/tree/2024.02.10) | None | Follow Linux-on-Litex instructions in how to build, read the [README](linux-on-litex-vexriscv/README.md). Clone the repo in CT-Key folder. |
| sphinx | 7.2.6 | PyPA | None | The package is included in the requirements.txt |
| meson | latest | PyPA | None | The package is included in the requirements.txt |
| sphinxcontrib-wavedrom | 2.1.1 | PyPA | None | The package is included in the requirements.txt |
| ninja | latest | PyPA | None | The package is included in the requirements.txt |

## How to build locally

### Creating development environment (WSL/Linux)

1. Create a root folder for your environment and cd into this folder. *Will call this the rootdev folder.*
2. Create folder "litex" and cd into the new folder
3. python3 -m venv .venv
4. source .venv/bin/activate
5. wget https://raw.githubusercontent.com/enjoy-digital/litex/master/litex_setup.py
6. python3 litex_setup.py --init --install --user
7. cd to rootdev
8. git clone --recursive https://github.com/Assured/CT-key.git
9. pip3 install -r CT-key/requirements.txt
10. cd to "CT-key/ctkey_package"
11. pip3 install --upgrade build; python3 -m build; pip3 install dist/*.whl
12. cd to rootdev
13. wget https://static.dev.sifive.com/dev-tools/freedom-tools/v2020.12/riscv64-unknown-elf-toolchain-10.2.0-2020.12.8-x86_64-linux-ubuntu14.tar.gz
14. tar -xf $PWD/riscv64-*.tar.gz
15. sudo mkdir /usr/local/riscv
16. sudo cp -r $PWD/riscv64-*/* /usr/local/riscv
17. wget https://github.com/YosysHQ/oss-cad-suite-build/releases/download/2025-01-08/oss-cad-suite-linux-x64-20250108.tgz
18. tar -xf $PWD/oss-cad-suite-linux-x64-20250108.tgz
19. sudo mv $PWD/oss-cad-suite /usr/local/
20. echo "export PATH=/usr/local/riscv:/usr/local/oss-cad-suite/bin:$PATH" >> ~/.profile
21. export PATH=/usr/local/riscv:/usr/local/oss-cad-suite/bin:$PATH

### Building and loading hardware

Need to have a working development environment before build can be done.

This will build the bitstream for the FPGA and generate documentation of registers that can be used. Will also compile the headers needed for development with register adresses and helper functions. For linux development, kernel modules in later versions of the linux kernel headers.

1. cd to rootdev
2. source litex/.venv/bin/activate
3. cd CT-Key/linux-on-litex-vexricv/
4. Check that Yosys and RISCV toolchain is in your path else run, export PATH=/usr/local/riscv:/usr/local/oss-cad-suite/bin:$PATH
5. python3 make.py --board orange_crab_ctkey --device 85F --doc --build
6. Put board in DFU, if needed, and connect board
7. python3 make.py --board orange_crab_ctkey --device 85F --load

### Building linux

This is how to build linux. The distrobution used is buildroot LTS version 2024.02.10.

(Default config "ctkey_defconfig" is under development which is targeted to generate a persistent version on sdcard, right now as ramdisk)

1. cd to rootdev/CT-key
2. git clone --checkout 2024.02.10 https://github.com/buildroot/buildroot
3. cd buildroot
4. make BR2_EXTERNAL=../linux-on-litex-vexricv/buildroot ctkey_defconfig
5. make

### Loading linux

This describes loading software from SD-card for Litex. Litex has various of other ways of loading software like serialboot, flashboot, romboot and ethernet boot. Read [this](https://github.com/enjoy-digital/litex/wiki/Load-Application-Code-To-CPU) documentation for further information.

1. Take an µSD-card and format as VFAT.
2. cd to rootdev/linux-on-litex-vexriscv/images
3. cp -L ./* sdcard/
4. Put sdcard in device and start

## Important files

**Configuration files for buildroot**

- linux-on-litex-vexriscv/buildroot - Top folder and contains all configuration on how buildroot should be built.
- linux-on-litex-vexriscv/buildroot/configs - Contains defconfigs for buildroot
- linux-on-litex-vexriscv/buildroot/board/\<defconfig_name\> - This folder contains board configuration for the defconfig used.
- linux-on-litex-vexriscv/buildroot/board/\<defconfig_name\>/rootfs_overlay/\<path\> - Path need to be a path on a linux system. It's an overlay for the rootfs with files to include in rootfs when generating image.
- linux-on-litex-vexriscv/buildroot/board/\<defconfig_name\>/genimage.cfg - Genimage config on how buildroot should generate an image after compilation.
- linux-on-litex-vexriscv/buildroot/board/\<defconfig_name\>/post-image.sh - Script that runs the genimage process.
- linux-on-litex-vexriscv/buildroot/board/\<defconfig_name\>/linux.config - How buildroot should configure linux

If you need to change any persistent settings in buildroot these need to be added here.

**Linux-on-litex-vexriscv files**

- linux-on-litex-vexriscv/boards.py - A file that contains all supported boards. New boards can be added but they need target SoC. See [ctkey_package/src/ctkey/targets] for examples on custom SoC based on existing.
- linux-on-litex-vexriscv/make.py - When adding custom boards or new, some changes may need to be done here.

**Python requirements**

- requirements.txt - Python requirements file. If additional python modules are required, add them in this file with a version if needed.

## Commits and PR

### Commits

All commits are required to be signed and follow standard commitlint-conventional rules when pushed. This is checked when code is pushed. Pre-commit hooks for commitlint can be configured.

### Pull-Requests

Rules for pull-requests before merging:

- Code Review
- Follow that project can be built
- Verify that it works on hardware