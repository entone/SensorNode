#ifndef AES_h
#define AES_h

#include "application.h"

void aes_128_encrypt(int value, unsigned char key[16], unsigned char output[64]);

#endif