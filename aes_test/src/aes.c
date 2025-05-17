#include "aes.h"

static void sleep(uint32_t ms)
{
    timer0_en_write(0);
    timer0_load_write(ms);
    timer0_update_value_write(1);
    timer0_en_write(1);
    while(timer0_value_read() != 0)
    {
        timer0_update_value_write(1);
    }
    // Disable the timer
    timer0_en_write(0);
    timer0_update_value_write(0);
}

static uint32_t read_word(uint8_t addr)
{
    uint32_t data = 0;
    uint32_t reg = 0;

    reg |= (addr << CSR_AES_CTRL_ADDR_OFFSET) | (1 << CSR_AES_CTRL_CS_OFFSET) | (1 << CSR_AES_CTRL_RST_OFFSET);
    aes_ctrl_write(reg);

    data = aes_output_read();

    #ifdef DEBUG
        reg = aes_ctrl_read();
        printf("[DEBUG][R]: addr = %02lx, rst = %01lx, cs = %01lx, we = %01lx, data = %08lx\n", (reg >> CSR_AES_CTRL_ADDR_OFFSET), reg & (1 << CSR_AES_CTRL_RST_OFFSET), reg & (1 << CSR_AES_CTRL_CS_OFFSET), reg & (1 << CSR_AES_CTRL_WE_OFFSET), data);
    #endif

    reg ^= (1 << CSR_AES_CTRL_CS_OFFSET);
    aes_ctrl_write(reg);

    #ifdef DEBUG
        reg = aes_ctrl_read();
        printf("[DEBUG][R]: addr = %02lx, rst = %01lx, cs = %01lx, we = %01lx\n", (reg >> CSR_AES_CTRL_ADDR_OFFSET), reg & (1 << CSR_AES_CTRL_RST_OFFSET), reg & (1 << CSR_AES_CTRL_CS_OFFSET), reg & (1 << CSR_AES_CTRL_WE_OFFSET));
    #endif

    return data;
}

static void write_word(uint8_t addr, const uint32_t data)
{
    uint32_t reg = 0;

    reg |= (addr << CSR_AES_CTRL_ADDR_OFFSET) | (1 << CSR_AES_CTRL_CS_OFFSET) | (1 << CSR_AES_CTRL_RST_OFFSET) | (1 << CSR_AES_CTRL_WE_OFFSET);

    aes_input_write(data);
    aes_ctrl_write(reg);

    #ifdef DEBUG
        reg = aes_ctrl_read();
        printf("[DEBUG][W]: addr = %02lx, rst = %01lx, cs = %01lx, we = %01lx, data = %08lx, ", (reg >> CSR_AES_CTRL_ADDR_OFFSET), reg & (1 << CSR_AES_CTRL_RST_OFFSET), reg & (1 << CSR_AES_CTRL_CS_OFFSET), reg & (1 << CSR_AES_CTRL_WE_OFFSET), data);
        reg = aes_input_read();
        printf("input = %08lx\n", reg);

        reg = aes_ctrl_read();
    #endif

    reg ^= (1 << CSR_AES_CTRL_CS_OFFSET) | (1 << CSR_AES_CTRL_WE_OFFSET);
    aes_ctrl_write(reg);
}

static void write_block(const uint32_t data[4])
{
    for(uint8_t i = 0; i < 4; i++)
    {
        write_word(ADDR_BLOCK0 + i, data[i]);
    }
}

static void read_block(uint32_t block[4])
{
    for (uint8_t i = 0; i < 4; i++)
    {
        block[i] = read_word(ADDR_BLOCK0 + i);
    }
}

static void write_key(const uint32_t data[8])
{
    for(uint8_t i = 0; i < 8; i++)
    {
        write_word(ADDR_KEY0 + i, data[i]);
    }
}

static void read_key(uint32_t key[8])
{
    for (uint8_t i = 0; i < 8; i++)
    {
        key[i] = read_word(ADDR_KEY0 + i);
    }
}

static void read_result(uint32_t result[4])
{
    for (uint8_t i = 0; i < 4; i++)
    {
        result[i] = read_word(ADDR_RESULT0 + i);
    }
}

void aes_reset(void)
{
    uint32_t reg = 0;

    #ifdef DEBUG
        reg = read_word(ADDR_STATUS);
        printf("[DEBUG][R]: ready = %01lx\n", reg & (1 << STATUS_READY_BIT));

        reg = 0;
    #endif

    printf("Resetting AES\n");
    aes_ctrl_write(reg);

    #ifdef DEBUG
        reg = aes_ctrl_read();
        printf("[DEBUG][R]: addr = %02lx, rst = %01lx, cs = %01lx, we = %01lx\n", (reg >> CSR_AES_CTRL_ADDR_OFFSET), reg & (1 << CSR_AES_CTRL_RST_OFFSET), reg & (1 << CSR_AES_CTRL_CS_OFFSET), reg & (1 << CSR_AES_CTRL_WE_OFFSET));
        reg = read_word(ADDR_STATUS);
        printf("[DEBUG][R]: ready = %01lx\n", reg & (1 << STATUS_READY_BIT));

        reg = 0;
    #endif

    reg |= (1 << CSR_AES_CTRL_RST_OFFSET);
    aes_ctrl_write(reg);

    #ifdef DEBUG
        reg = aes_ctrl_read();
        printf("[DEBUG][R]: addr = %02lx, rst = %01lx, cs = %01lx, we = %01lx\n", (reg >> CSR_AES_CTRL_ADDR_OFFSET), reg & (1 << CSR_AES_CTRL_RST_OFFSET), reg & (1 << CSR_AES_CTRL_CS_OFFSET), reg & (1 << CSR_AES_CTRL_WE_OFFSET));
        reg = read_word(ADDR_STATUS);
        printf("[DEBUG][R]: ready = %01lx\n", reg & (1 << STATUS_READY_BIT));

        reg = 0;
    #endif

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

void aes_init_key(const uint32_t key[8], uint8_t key_size)
{
    printf("Initializing AES key\n");

    write_key(key);

    if(key_size) {
        write_word(ADDR_CONFIG, (uint8_t) 0x02);
    } else {
        write_word(ADDR_CONFIG, (uint8_t) 0x00);
    }
    
    write_word(ADDR_CTRL, (uint8_t) 0x01);
}

void dump_state(void)
{
    uint32_t ctrl = read_word(ADDR_CTRL);

    printf("AES state:\n");
    printf("Control Register: init = %01lx, next = %01lx\n", ctrl & (1 << CTRL_INIT_BIT), ctrl & (1 << CTRL_NEXT_BIT));
    printf("Configuration Register: encdec = %01lx, length = %01lx\n", ctrl & (1 << CTRL_ENCDEC_BIT), ctrl & (1 << CTRL_KEYLEN_BIT));
    printf("Status Register: ready = %01lx\n", read_word(ADDR_STATUS) & (1 << STATUS_READY_BIT));
    
    uint32_t block[4];
    uint32_t key[8];
    read_block(block);
    read_key(key);
    printf("Block: %08lx%08lx%08lx%08lx\n", block[0], block[1], block[2], block[3]);
    printf("Key: %08lx%08lx%08lx%08lx%08lx%08lx%08lx%08lx\n\n", 
           key[0], key[1], key[2], key[3], key[4], key[5], key[6], key[7]);
}

bool ecb_mode_single_block_test(uint8_t tc_number, uint8_t encdec, const uint32_t key[8], uint8_t key_len, const uint32_t plaintext[4], const uint32_t expected[4])
{
    printf("Running test case %d\n", tc_number);

    aes_init_key(key, key_len);
    write_block(plaintext);
    dump_state();

    write_word(ADDR_CONFIG, (uint8_t) (0x00 + (key_len << 1) + encdec));
    write_word(ADDR_CTRL, (uint8_t) 0x02);

    sleep(500); // Wait for the AES operation to complete

    uint32_t result[4];
    read_result(result);

    if (memcmp(result, expected, sizeof(result)) == 0) {
        printf("Test case %d passed\n\n", tc_number);
        return true;
    } else {
        printf("Test case %d failed\n", tc_number);
        printf("Expected: %08lx%08lx%08lx%08lx\n", expected[0], expected[1], expected[2], expected[3]);
        printf("Got: %08lx%08lx%08lx%08lx\n\n", result[0], result[1], result[2], result[3]);
        
        return false;
    }
}

