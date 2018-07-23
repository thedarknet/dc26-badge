#include <stdio.h> 
#include "sha256.h"

#define RotL(a,b) (((a) << (b)) | ((a) >> (32-(b))))
#define RotR(a,b) (((a) >> (b)) | ((a) << (32-(b))))
#define CH(x,y,z) (((x) & (y)) ^ (~(x) & (z)))
#define MAJ(x,y,z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))

#define EP0(x) (RotR(x,2) ^ RotR(x,13) ^ RotR(x,22))
#define EP1(x) (RotR(x,6) ^ RotR(x,11) ^ RotR(x,25))

#define SIG0(x) (RotR(x,7) ^ RotR(x,18) ^ ((x) >> 3))
#define SIG1(x) (RotR(x,17) ^ RotR(x,19) ^ ((x) >> 10))

uint32 k[64] = {
	0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
	0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
	0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
	0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
	0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
	0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
	0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
	0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
};

void sha256_init(ShaOBJ *ctx){  
	ctx->state[0] = 0x6a09e667;
	ctx->state[1] = 0xbb67ae85;
	ctx->state[2] = 0x3c6ef372;
	ctx->state[3] = 0xa54ff53a;
	ctx->state[4] = 0x510e527f;
	ctx->state[5] = 0x9b05688c;
	ctx->state[6] = 0x1f83d9ab;
	ctx->state[7] = 0x5be0cd19;
	ctx->data_len = 0;
	ctx->bit_len  = 0;
}

void sha256_update(ShaOBJ* ctx){
	uint32 a,b,c,d,e,f,g,h;
	uint32 t1,t2;
	uint32 w[64];
	uint32 i,j;

	//W(j) = M(j)
	j=0;
	for(i=0;i<16;++i){
		w[i] = (ctx->data[j] << 24) | (ctx->data[j+1] << 16) | (ctx->data[j+2] << 8) | ctx->data[j+3];
		j+=4;
	}

	//Compute W For J = 16..63
	for(;i<64;++i){
		w[i] = SIG1(w[i-2]) + w[i-7] + SIG0(w[i-15]) + w[i-16];
	}

	//Initialize Registers
	a = ctx->state[0];
	b = ctx->state[1];
	c = ctx->state[2];
	d = ctx->state[3];
	e = ctx->state[4];
	f = ctx->state[5];
	g = ctx->state[6];
	h = ctx->state[7];

	//Main Loop
	for(j=0;j<64;j++){
		//Compute t1 and t2
		t1 = h + EP1(e) + CH(e,f,g) + k[j] + w[j];
		t2 = EP0(a) + MAJ(a,b,c);

		//Move Registers Along
		h = g;
		g = f;
		f = e;
		e = d + t1;
		d = c;
		c = b;
		b = a;
		a = t1 + t2;
	}

	//Update State
	ctx->state[0] += a;
	ctx->state[1] += b;
	ctx->state[2] += c;
	ctx->state[3] += d;
	ctx->state[4] += e;
	ctx->state[5] += f;
	ctx->state[6] += g;
	ctx->state[7] += h;

}

void sha256_add(ShaOBJ* ctx, const unsigned char* msg, uint32 len){
	uint32 off = 0;
	ctx->bit_len += len*8;
	while(off<len){
		ctx->data[ctx->data_len] = *(msg+off);
		ctx->data_len ++;
		off ++;
		if(ctx->data_len==64){
			sha256_update(ctx);
			ctx->data_len = 0;
		}
	}
}

void sha256_digest(ShaOBJ* ctx, unsigned char hash[]){
	uchar out[128];

	//Create Final Block
	uint32 size = ctx->data_len + (64-(ctx->data_len%65)); //Size Of Last Block(s) 64 or 128

	//Copy Current Data
	for(uint32 i=0;i<ctx->data_len;i++){
		out[i] = ctx->data[i];
	}

	//Zero Out Buffer
	for(uint32 i=ctx->data_len;i<size;i++){
		out[i] = 0x00;
	}

	//Set 1 Flag (BYTE = 128)
	out[ctx->data_len] = 0x80;

	//Write Final Bit Length
	for(int i=0;i<8;i++){
		out[size-i-1] = ctx->bit_len >> i*8;
	}

	//Clear CTX Data Buffer (Move To The Start, It Will Override By It Self)
	ctx->data_len = 0;

	//Update
	sha256_add(ctx,(uchar*)&out,size);

	//Transform From Little Endian To Big Endian
	for(int n=0;n<8;n++){
		for(int i=3;i>=0;i--){
			//printf("%x - ",ctx->state[i]);
			hash[n*4+(3-i)] = (ctx->state[n] >> i*8) & 0xFF;
		}
	}

	//Result Should Now Be Stored In Hash
}
