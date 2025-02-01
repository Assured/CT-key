#include <stdio.h>
#include "aes.h"

int main(void)
{
    char buf[13];
    aes_reset();
    aes_getinfo(buf);

    printf("AES core name: %s\n", buf);

    return 0;
}