#ifndef __AES_H
#define __AES_H

// Address map for various registers
#define ADDR_NAME0 0x00 // Address for the first part of the core name
#define ADDR_NAME1 0x01 // Address for the second part of the core name
#define ADDR_VERSION 0x02 // Address for the core version

// Control register and its bit positions
#define ADDR_CTRL 0x08 // Address for the control register
#define CTRL_INIT_BIT 0 // Bit position for the initialization control
#define CTRL_NEXT_BIT 1 // Bit position for the next control

// Status register and its bit positions
#define ADDR_STATUS 0x09 // Address for the status register
#define STATUS_READY_BIT 0 // Bit position indicating if the core is ready
//#define STATUS_VALID_BIT 1 // Bit position indicating if the output is valid

// Configuration register and its bit positions
#define ADDR_CONFIG 0x0a // Address for the configuration register
#define CTRL_ENCDEC_BIT 0 // Bit position for encryption/decryption control
#define CTRL_KEYLEN_BIT 1 // Bit position for key length control

// Key registers
#define ADDR_KEY0 0x10 // Address for the first key register
#define ADDR_KEY7 0x17 // Address for the last key register

// Block registers
#define ADDR_BLOCK0 0x20 // Address for the first block register
#define ADDR_BLOCK3 0x23 // Address for the last block register

// Result registers
#define ADDR_RESULT0 0x30 // Address for the first result register
#define ADDR_RESULT3 0x33 // Address for the last result register

// void aes_load_key(unsigned char * key, bool size_256);
// void aes_load_data(unsigned char * data, short int size);
// void aes_encrypt_data(char * data, short int size);
// void aes_decrypt_data(char * data, short int size);
void aes_reset(void);
void aes_getinfo(char * buf);

#endif


