#ifndef __AES_H
#define __AES_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <generated/csr.h>
#include <generated/mem.h>
#include <generated/soc.h>

// Address map for various registers
//#define DEBUG
#define ADDR_NAME0 0x00 // Address for the first part of the core name
#define ADDR_NAME1 0x01 // Address for the second part of the core name
#define ADDR_VERSION 0x02 // Address for the core version

// Control register and its bit positions
#define ADDR_CTRL 0x08 // Address for the control register
#define CTRL_INIT_BIT 0 // Bit position for the initialization control
#define CTRL_NEXT_BIT 1 // Bit position for the next control
#define CTRL_ENCDEC_BIT 2 // Bit position for encryption/decryption control
#define CTRL_KEYLEN_BIT 3 // Bit position for key length control

// Status register and its bit positions
#define ADDR_STATUS 0x09 // Address for the status register
#define STATUS_READY_BIT 0 // Bit position indicating if the core is ready
#define STATUS_VALID_BIT 1 // Bit position indicating if the output is valid

// Configuration register and its bit positions
#define ADDR_CONFIG 0x0a // Address for the configuration register

// Key registers
#define ADDR_KEY0 0x10 // Address for the first key register
#define ADDR_KEY1 0x11 // Address for the second key register
#define ADDR_KEY2 0x12 // Address for the third key register
#define ADDR_KEY3 0x13 // Address for the fourth key register
#define ADDR_KEY4 0x14 // Address for the fifth key register
#define ADDR_KEY5 0x15 // Address for the sixth key register
#define ADDR_KEY6 0x16 // Address for the seventh key register
#define ADDR_KEY7 0x17 // Address for the last key register

// Block registers
#define ADDR_BLOCK0 0x20 // Address for the first block register
#define ADDR_BLOCK1 0x21 // Address for the second block register
#define ADDR_BLOCK2 0x22 // Address for the third block register
#define ADDR_BLOCK3 0x23 // Address for the last block register

// Result registers
#define ADDR_RESULT0 0x30 // Address for the first result register
#define ADDR_RESULT1 0x31 // Address for the second result register
#define ADDR_RESULT2 0x32 // Address for the third result register
#define ADDR_RESULT3 0x33 // Address for the last result register

// Key size definitions
#define AES_KEY_SIZE_128 0
#define AES_KEY_SIZE_256 1

// AES decryption and encryption mode
#define AES_DECIPHER 0
#define AES_ENCIPHER 1

// Timer defines
#define US_TO_TICKS(x) ((x) * (CONFIG_CLOCK_FREQUENCY / 1000000))

// Function prototypes
void aes_init_key(const uint32_t key[8], uint8_t key_size);
void aes_reset(void);
void aes_getinfo(char * buf);
bool aes_ready(void);
bool ecb_mode_single_block_test(uint8_t tc_number, uint8_t encdec, const uint32_t key[8], uint8_t key_len, const uint32_t plaintext[4], const uint32_t expected[4]);
void dump_state(void);
void aes_start_operation(uint8_t encdec, uint8_t key_len);
#endif


