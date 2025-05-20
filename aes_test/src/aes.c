#include "aes.h"

static void sleep(uint32_t ticks)
{
    timer0_en_write(0);
    timer0_load_write(ticks);
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

    reg ^= (1 << CSR_AES_CTRL_CS_OFFSET);
    aes_ctrl_write(reg);

    return data;
}

static void write_word(uint8_t addr, const uint32_t data)
{
    uint32_t reg = 0;

    reg |= (addr << CSR_AES_CTRL_ADDR_OFFSET) | (1 << CSR_AES_CTRL_CS_OFFSET) | (1 << CSR_AES_CTRL_RST_OFFSET) | (1 << CSR_AES_CTRL_WE_OFFSET);

    aes_input_write(data);
    aes_ctrl_write(reg);

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

static void write_key(const uint32_t data[8])
{
    for(uint8_t i = 0; i < 8; i++)
    {
        write_word(ADDR_KEY0 + i, data[i]);
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

    printf("Resetting AES\n");
    aes_ctrl_write(reg);

    reg |= (1 << CSR_AES_CTRL_RST_OFFSET);
    aes_ctrl_write(reg);

    while(! aes_ready()){}
    
    printf("AES reset completed\n");
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
        status = 1;
    }

    return status;
}

void aes_init_key(const uint32_t key[8], uint8_t key_size)
{
    printf("Initializing AES key\n");

    write_key(key);

    // Set the key size in the configuration register
    if(key_size) {
        write_word(ADDR_CONFIG, (uint8_t) 0x02);
    } else {
        write_word(ADDR_CONFIG, (uint8_t) 0x00);
    }
    
    // Set the control register to initialize the key
    write_word(ADDR_CTRL, (uint8_t) 0x01);

    // Wait for the AES operation to complete
    while(! aes_ready()) {
        sleep(US_TO_TICKS(1));
    }

    printf("AES key initialization completed\n");
}

void dump_state(void)
{
    uint32_t ctrl = read_word(ADDR_CTRL);
    uint32_t status = read_word(ADDR_STATUS);

    printf("AES state:\n");
    printf("Control Register: init = %01lx, next = %01lx\n", (ctrl >> CTRL_INIT_BIT) & 1, (ctrl >> CTRL_NEXT_BIT) & 1);
    printf("Configuration Register: encdec = %01lx, length = %01lx\n", (ctrl >> CTRL_ENCDEC_BIT) & 1, (ctrl >> CTRL_KEYLEN_BIT) & 1);
    printf("Status Register: valid = %01lx, ready = %01lx\n", (status >> STATUS_VALID_BIT) & 1, (status >> STATUS_READY_BIT) & 1);
}

void aes_start_operation(uint8_t encdec, uint8_t key_len) {
    printf("Starting AES operation\n");

    write_word(ADDR_CONFIG, (uint8_t) (0x00 + (key_len << 1) + encdec));
    write_word(ADDR_CTRL, (uint8_t) 0x02);

    // Wait for the AES operation to complete
    while(! aes_ready()) {
        sleep(US_TO_TICKS(1));
    }

    printf("AES operation completed\n");
}

bool ecb_mode_single_block_test(uint8_t tc_number, uint8_t encdec, const uint32_t key[8], uint8_t key_len, const uint32_t plaintext[4], const uint32_t expected[4])
{
    printf("Running test case %d\n", tc_number);

    aes_init_key(key, key_len);
    write_block(plaintext);
    dump_state();

    aes_start_operation(encdec, key_len);
    dump_state();

    // Read the result
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

