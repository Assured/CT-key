#include "aes.h"
#include <string.h>
#include <generated/csr.h>

void aes_reset(void)
{
    uint32_t data = aes_CTRL_REG_read();
    data ^= (1 << CSR_AES_CTRL_REG_RST_OFFSET);
    aes_CTRL_REG_write(data);

    while(! (data & (1 << CSR_AES_CTRL_REG_RST_OFFSET)))
    {
        data = aes_CTRL_REG_read();
    }
}

void aes_getinfo(char * buf)
{
    memset(buf, '\n', 13);

    for(int i = 0; i < 3; i++)
    {
        uint32_t data = aes_CTRL_REG_read();
        data |= (0 << CSR_AES_CTRL_REG_ADDR_OFFSET) | (1 << CSR_AES_CTRL_REG_CS_OFFSET);
        aes_CTRL_REG_write(data);

        data = aes_READ_REG_read();
        memcpy(&buf[i*4], &data, 4);

        data ^= (1 << CSR_AES_CTRL_REG_CS_OFFSET);
        aes_CTRL_REG_write(data);
    }
}