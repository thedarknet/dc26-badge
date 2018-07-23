#ifndef SHA256_H
#define SHA256_H


#define uchar unsigned char

#define uint_64 long long unsigned int

#define uint32 unsigned int

struct ShaOBJ {
	uint32 state[8];
	uint_64 bit_len;
	uchar data[64];
	uint32 data_len;
}; 

void sha256_init(ShaOBJ *ctx);
void sha256_update(ShaOBJ* ctx);
void sha256_add(ShaOBJ* ctx, const unsigned char* msg, uint32 len);
void sha256_digest(ShaOBJ* ctx, unsigned char hash[]);

#endif
