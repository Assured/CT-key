#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <irq.h>
#include <libbase/uart.h>
#include <libbase/console.h>

#include "aes.h"

// AES test vectors
const uint32_t nist_aes128_key1[8] = {
    0x2b7e1516,
    0x28aed2a6,
    0xabf71588,
    0x09cf4f3c,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000
};

const uint32_t nist_aes128_key2[8] = {
    0x00010203,
    0x04050607,
    0x08090a0b,
    0x0c0d0e0f,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000
};

const uint32_t nist_aes256_key1[8] = {
    0x603deb10,
    0x15ca71be,
    0x2b73aef0,
    0x857d7781,
    0x1f352c07,
    0x3b6108d7,
    0x2d9810a3,
    0x0914dff4
};

const uint32_t nist_aes256_key2[8] = {
    0x00010203,
    0x04050607,
    0x08090a0b,
    0x0c0d0e0f,
    0x10111213,
    0x14151617,
    0x18191a1b,
    0x1c1d1e1f
};

const uint32_t nist_plaintext0[4] = {
    0x6bc1bee2,
    0x2e409f96,
    0x93d7e117,
    0x393172a0
};

const uint32_t nist_plaintext1[4] = {
    0xae2d8a57,
    0x1e03ac9c,
    0x9eb76fac,
    0x45af8e51
};

const uint32_t nist_plaintext2[4] = {
    0x30c81c46,
    0xa35ce411,
    0xe5fbc119,
    0x1a0a52ef
};

const uint32_t nist_plaintext3[4] = {
    0xf69f2445,
    0xdf4f9b17,
    0xad2b417b,
    0xe66c3710
};

const uint32_t nist_plaintext4[4] = {
    0x00112233,
    0x44556677,
    0x8899aabb,
    0xccddeeff
};

const uint32_t nist_ecb_128_enc_expected0[4] = {
    0x3ad77bb4,
    0x0d7a3660,
    0xa89ecaf3,
    0x2466ef97
};

const uint32_t nist_ecb_128_enc_expected1[4] = {
    0xf5d3d585,
    0x03b9699d,
    0xe785895a,
    0x96fdbaaf
};

const uint32_t nist_ecb_128_enc_expected2[4] = {
    0x43b1cd7f,
    0x598ece23,
    0x881b00e3,
    0xed030688
};

const uint32_t nist_ecb_128_enc_expected3[4] = {
    0x7b0c785e,
    0x27e8ad3f,
    0x82232071,
    0x04725dd4
};

const uint32_t nist_ecb_128_enc_expected4[4] = {
    0x69c4e0d8,
    0x6a7b0430,
    0xd8cdb780,
    0x70b4c55a
};

const uint32_t nist_ecb_256_enc_expected0[4] = {
    0xf3eed1bd,
    0xb5d2a03c,
    0x064b5a7e,
    0x3db181f8
};

const uint32_t nist_ecb_256_enc_expected1[4] = {
    0x591ccb10,
    0xd410ed26,
    0xdc5ba74a,
    0x31362870
};

const uint32_t nist_ecb_256_enc_expected2[4] = {
    0xb6ed21b9,
    0x9ca6f4f9,
    0xf153e7b1,
    0xbeafed1d
};

const uint32_t nist_ecb_256_enc_expected3[4] = {
    0x23304b7a,
    0x39f9f3ff,
    0x067d8d8f,
    0x9e24ecc7
};

const uint32_t nist_ecb_256_enc_expected4[4] = {
    0x8ea2b7ca,
    0x516745bf,
    0xeafc4990,
    0x4b496089
};

int main(void)
{
#ifdef CONFIG_CPU_HAS_INTERRUPT
	irq_setmask(0);
	irq_setie(1);
#endif
    // Initialize the UART for console output
    uart_init();

    char buf[13];
    aes_reset();
    aes_getinfo(buf);

    printf("AES core name: %s\n", buf);

	ecb_mode_single_block_test(1, AES_CIPHER, nist_aes128_key1, AES_KEY_SIZE_128, nist_plaintext0, nist_ecb_128_enc_expected0);

    return 0;
}