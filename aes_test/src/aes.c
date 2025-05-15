#include "aes.h"

static uint32_t read_word(uint8_t addr)
{
    uint32_t data = 0;
    uint32_t reg = 0;

    reg |= (addr << CSR_AES_CTRL_REG_ADDR_OFFSET) | (1 << CSR_AES_CTRL_REG_CS_OFFSET) | (1 << CSR_AES_CTRL_REG_RST_OFFSET);
    aes_CTRL_REG_write(reg);

    data = aes_READ_REG_read();

    reg ^= (1 << CSR_AES_CTRL_REG_CS_OFFSET);
    aes_CTRL_REG_write(reg);

    return data;
}

static void write_word(uint8_t addr, const uint32_t data)
{
    uint32_t reg = 0;
    aes_WRITE_REG_write(data);

    #ifdef DEBUG
        printf("[DEBUG]: Writing %08lx to address 0x%02x\n", data, addr);
    #endif

    reg |= (addr << CSR_AES_CTRL_REG_ADDR_OFFSET) | (1 << CSR_AES_CTRL_REG_CS_OFFSET) | (1 << CSR_AES_CTRL_REG_RST_OFFSET) | (1 << CSR_AES_CTRL_REG_WE_OFFSET);
    aes_CTRL_REG_write(reg);

    reg ^= ((1 << CSR_AES_CTRL_REG_CS_OFFSET) | (1 << CSR_AES_CTRL_REG_WE_OFFSET));
    aes_CTRL_REG_write(reg);

    #ifdef DEBUG
        printf("[DEBUG]: Wrote %08lx to address 0x%02x\n", read_word(addr), addr);
    #endif
}

static void write_block(const uint32_t data[4])
{
    for (uint8_t i = 0; i < 4; i++)
    {
        write_word(ADDR_BLOCK0 + i, data[i]);
    }
}


void aes_reset(void)
{
    uint32_t data = 0;
    data |= (1 << CSR_AES_CTRL_REG_RST_OFFSET);

    printf("Resetting AES\n");
    aes_CTRL_REG_write(data);

    printf("Waiting for AES to reset\n");
    while(! aes_ready()){}
} 

void aes_getinfo(char * buf)
{
    memset(buf, '\0', 13);
    uint32_t data = 0;

    for(uint8_t i = 0; i < 3; i++)
    {
        data = read_word(ADDR_NAME0 + i);

        // Manually reverse the byte order of data
        data = ((data >> 24) & 0x000000FF) |  // Move byte 3 to byte 0
               ((data >> 8)  & 0x0000FF00) |  // Move byte 2 to byte 1
               ((data << 8)  & 0x00FF0000) |  // Move byte 1 to byte 2
               ((data << 24) & 0xFF000000);   // Move byte 0 to byte 3

        memcpy(&buf[i*4], &data, 4);
    }
}

bool aes_ready(void)
{
    bool status = 0;

    if(read_word(ADDR_STATUS) & (1 << STATUS_READY_BIT)) {
        printf("AES is ready\n");
        status = 1;
    } else {
        printf("AES is not ready\n");
    }

    return status;
}

void aes_init_key(const uint32_t key[8], bool key_size)
{
    for (uint8_t i = 0; i < 8; i++)
    {
        write_word(ADDR_KEY0 + i, key[i]);
    }

    if(key_size) {
        write_word(ADDR_CONFIG, (1 << CTRL_KEYLEN_BIT));
    } else {
        write_word(ADDR_CONFIG, 0);
    }
    
    write_word(ADDR_CTRL, (1 << CTRL_INIT_BIT));
}

void dump_state(void)
{
    uint32_t ctrl = read_word(ADDR_CTRL);
    uint32_t config = read_word(ADDR_CONFIG);

    printf("AES state:\n");
    printf("Control Register: init = %01lx, next = %01lx\n", ctrl & (1 << CTRL_INIT_BIT), ctrl & (1 << CTRL_NEXT_BIT));
    printf("Configuration Register: encdec = %01lx, length = %01lx\n", config & (1 << CTRL_ENCDEC_BIT), config & (1 << CTRL_KEYLEN_BIT));
    printf("Block: %08lx%08lx%08lx%08lx\n", 
           read_word(ADDR_BLOCK0),
           read_word(ADDR_BLOCK1),
           read_word(ADDR_BLOCK2),
           read_word(ADDR_BLOCK3));
}

void read_result(uint32_t result[4])
{
    for (uint8_t i = 0; i < 4; i++)
    {
        result[i] = read_word(ADDR_RESULT0 + i);
    }
}

bool ecb_mode_single_block_test(uint8_t tc_number, bool encdec, const uint32_t key[8], bool key_len, const uint32_t plaintext[4], const uint32_t expected[4])
{
    printf("Running test case %d\n", tc_number);

    aes_init_key(key, key_len);
    write_block(plaintext);
    dump_state();

    write_word(ADDR_CONFIG, ((key_len << CTRL_KEYLEN_BIT) | (encdec << CTRL_ENCDEC_BIT)));
    write_word(ADDR_CTRL, (1 << CTRL_NEXT_BIT));

    uint32_t result[4];
    read_result(result);

    if (memcmp(result, expected, sizeof(result)) == 0) {
        printf("Test case %d passed\n", tc_number);
        return true;
    } else {
        printf("Test case %d failed\n", tc_number);
        printf("Expected: %08lx%08lx%08lx%08lx\n", expected[0], expected[1], expected[2], expected[3]);
        printf("Got: %08lx%08lx%08lx%08lx\n", result[0], result[1], result[2], result[3]);
        
        return false;
    }
}

