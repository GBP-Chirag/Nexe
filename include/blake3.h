/*  This is the project form creator Chirag Balakrishna.
    This project is licensed under GPLv3
    Thats it ! Feel free to contribute !! Happy Journey 😀
*/

/*
 * Minimal, portable BLAKE3 implementation for Nexe.
 * Header-only: no separate library or link step is needed.
 * BLAKE3 is CC0/public-domain: https://github.com/BLAKE3-team/BLAKE3
 */
#ifndef NEXE_BLAKE3_H
#define NEXE_BLAKE3_H

#include <stddef.h> // for size_t
#include <stdint.h> // for uint*_t
#include <string.h> // for memcpy, strlen

#ifdef __cplusplus
extern "C" {
#endif
/*These are the constants that are used in the blake3 hashing algorithm. They define the output length, key length, block length, chunk length, and maximum depth of the hashing algorithm. 
These constants are used throughout the implementation of the blake3 hashing algorithm to ensure that the correct parameters are used for hashing and to maintain consistency in the output of the algorithm.
*/ 
#define BLAKE3_OUT_LEN 32 // Output length in bytes
#define BLAKE3_KEY_LEN 32
#define BLAKE3_BLOCK_LEN 64
#define BLAKE3_CHUNK_LEN 1024
#define BLAKE3_MAX_DEPTH 54

/* These are the data structures that are used in the blake3 hashing algorithm.
 * They define the state of the hashing process and the output of the algorithm.
 */
typedef struct {
  uint32_t cv[8]; uint64_t chunk_counter; uint8_t buf[BLAKE3_BLOCK_LEN]; // Buffer for the current chunk
  uint8_t buf_len, blocks_compressed, flags; // Length of the buffer, number of blocks compressed, and flags for the current chunk
} blake3_chunk_state; // State of the current chunk being processed
// Hasher state
typedef struct {
  uint32_t key[8]; blake3_chunk_state chunk; uint8_t cv_stack_len;
  uint8_t cv_stack[BLAKE3_MAX_DEPTH * BLAKE3_OUT_LEN];
} blake3_hasher;
typedef struct { uint32_t input_cv[8], block[16]; uint64_t counter; uint32_t block_len, flags; } blake3_output;
enum { BLAKE3_CHUNK_START=1, BLAKE3_CHUNK_END=2, BLAKE3_PARENT=4, BLAKE3_ROOT=8,
       BLAKE3_KEYED_HASH=16, BLAKE3_DERIVE_KEY_CONTEXT=32, BLAKE3_DERIVE_KEY_MATERIAL=64 };
static const uint32_t blake3_iv[8]={0x6A09E667u,0xBB67AE85u,0x3C6EF372u,0xA54FF53Au,0x510E527Fu,0x9B05688Cu,0x1F83D9ABu,0x5BE0CD19u};
static const uint8_t blake3_msg_permutation[16]={2,6,3,10,7,0,4,13,1,11,12,5,9,14,15,8};
static inline uint32_t blake3_rotr32(uint32_t x,unsigned n){return(x>>n)|(x<<(32-n));}
static inline uint32_t blake3_load32(const uint8_t*p){return(uint32_t)p[0]|((uint32_t)p[1]<<8)|((uint32_t)p[2]<<16)|((uint32_t)p[3]<<24);}
static inline void blake3_store32(uint8_t*p,uint32_t x){p[0]=(uint8_t)x;p[1]=(uint8_t)(x>>8);p[2]=(uint8_t)(x>>16);p[3]=(uint8_t)(x>>24);}
static inline void blake3_g(uint32_t s[16],unsigned a,unsigned b,unsigned c,unsigned d,uint32_t x,uint32_t y){s[a]+=s[b]+x;s[d]=blake3_rotr32(s[d]^s[a],16);s[c]+=s[d];s[b]=blake3_rotr32(s[b]^s[c],12);s[a]+=s[b]+y;s[d]=blake3_rotr32(s[d]^s[a],8);s[c]+=s[d];s[b]=blake3_rotr32(s[b]^s[c],7);}
static inline void blake3_round(uint32_t s[16],const uint32_t m[16]){blake3_g(s,0,4,8,12,m[0],m[1]);blake3_g(s,1,5,9,13,m[2],m[3]);blake3_g(s,2,6,10,14,m[4],m[5]);blake3_g(s,3,7,11,15,m[6],m[7]);blake3_g(s,0,5,10,15,m[8],m[9]);blake3_g(s,1,6,11,12,m[10],m[11]);blake3_g(s,2,7,8,13,m[12],m[13]);blake3_g(s,3,4,9,14,m[14],m[15]);}
static inline void blake3_permute(uint32_t m[16]){uint32_t c[16];unsigned i;for(i=0;i<16;i++)c[i]=m[blake3_msg_permutation[i]];memcpy(m,c,sizeof(c));}
static inline void blake3_compress(const uint32_t cv[8],const uint32_t b[16],uint64_t counter,uint32_t len,uint32_t flags,uint32_t out[16]){uint32_t s[16],m[16];unsigned i;for(i=0;i<8;i++)s[i]=cv[i];for(i=0;i<4;i++)s[i+8]=blake3_iv[i];s[12]=(uint32_t)counter;s[13]=(uint32_t)(counter>>32);s[14]=len;s[15]=flags;memcpy(m,b,sizeof(m));for(i=0;i<7;i++){blake3_round(s,m);if(i!=6)blake3_permute(m);}for(i=0;i<8;i++){out[i]=s[i]^s[i+8];out[i+8]=s[i+8]^cv[i];}}
static inline void blake3_words(const uint8_t b[64],uint32_t w[16]){unsigned i;for(i=0;i<16;i++)w[i]=blake3_load32(&b[4*i]);}
static inline uint32_t blake3_chunk_len(const blake3_chunk_state*s){return(uint32_t)s->blocks_compressed*64+s->buf_len;}
static inline void blake3_chunk_init(blake3_chunk_state*s,const uint32_t key[8],uint64_t counter,uint8_t flags){memcpy(s->cv,key,32);s->chunk_counter=counter;s->buf_len=s->blocks_compressed=0;s->flags=flags;}
static inline void blake3_chunk_update(blake3_chunk_state*s,const uint8_t*in,size_t n){while(n){if(s->buf_len==64){uint32_t b[16],o[16],f=s->flags|(s->blocks_compressed==0?BLAKE3_CHUNK_START:0);blake3_words(s->buf,b);blake3_compress(s->cv,b,s->chunk_counter,64,f,o);memcpy(s->cv,o,32);s->blocks_compressed++;s->buf_len=0;}{size_t want=64-s->buf_len,take=n<want?n:want;memcpy(s->buf+s->buf_len,in,take);s->buf_len=(uint8_t)(s->buf_len+take);in+=take;n-=take;}}}
static inline blake3_output blake3_chunk_output(const blake3_chunk_state*s){blake3_output o;uint8_t b[64]={0};memcpy(o.input_cv,s->cv,32);memcpy(b,s->buf,s->buf_len);blake3_words(b,o.block);o.counter=s->chunk_counter;o.block_len=s->buf_len;o.flags=s->flags|BLAKE3_CHUNK_END|(s->blocks_compressed==0?BLAKE3_CHUNK_START:0);return o;}
static inline void blake3_output_cv(const blake3_output*o,uint32_t cv[8]){uint32_t x[16];blake3_compress(o->input_cv,o->block,o->counter,o->block_len,o->flags,x);memcpy(cv,x,32);}
static inline void blake3_root_bytes(const blake3_output*o,uint64_t seek,uint8_t*out,size_t n){uint64_t ctr=seek/64;size_t off=(size_t)(seek%64);while(n){uint32_t w[16];uint8_t b[64];size_t i,take;blake3_compress(o->input_cv,o->block,ctr,o->block_len,o->flags|BLAKE3_ROOT,w);for(i=0;i<16;i++)blake3_store32(b+4*i,w[i]);take=n<64-off?n:64-off;memcpy(out,b+off,take);out+=take;n-=take;off=0;ctr++;}}
static inline blake3_output blake3_parent_output(const uint32_t l[8],const uint32_t r[8],const uint32_t key[8],uint8_t flags){blake3_output o;memcpy(o.input_cv,key,32);memcpy(o.block,l,32);memcpy(o.block+8,r,32);o.counter=0;o.block_len=64;o.flags=(uint32_t)flags|BLAKE3_PARENT;return o;}
static inline void blake3_parent_cv(const uint32_t l[8],const uint32_t r[8],const uint32_t key[8],uint8_t flags,uint32_t out[8]){blake3_output o=blake3_parent_output(l,r,key,flags);blake3_output_cv(&o,out);}
static inline void blake3_hasher_init_internal(blake3_hasher*s,const uint32_t key[8],uint8_t flags){memcpy(s->key,key,32);blake3_chunk_init(&s->chunk,key,0,flags);s->cv_stack_len=0;}
static inline void blake3_hasher_init(blake3_hasher*s){blake3_hasher_init_internal(s,blake3_iv,0);}
static inline void blake3_hasher_init_keyed(blake3_hasher*s,const uint8_t key[32]){uint32_t w[8];unsigned i;for(i=0;i<8;i++)w[i]=blake3_load32(key+4*i);blake3_hasher_init_internal(s,w,BLAKE3_KEYED_HASH);}
static inline void blake3_push(blake3_hasher*s,const uint32_t cv[8]){memcpy(s->cv_stack+(size_t)s->cv_stack_len*32,cv,32);s->cv_stack_len++;}
static inline void blake3_pop(blake3_hasher*s,uint32_t cv[8]){s->cv_stack_len--;memcpy(cv,s->cv_stack+(size_t)s->cv_stack_len*32,32);}
static inline void blake3_add_chunk_cv(blake3_hasher*s,const uint32_t new_cv[8],uint64_t total){uint32_t cv[8];memcpy(cv,new_cv,32);while((total&1)==0){uint32_t left[8];blake3_pop(s,left);blake3_parent_cv(left,cv,s->key,s->chunk.flags,cv);total>>=1;}blake3_push(s,cv);}
static inline void blake3_hasher_update(blake3_hasher*s,const void*input,size_t n){const uint8_t*in=(const uint8_t*)input;while(n){if(blake3_chunk_len(&s->chunk)==1024){uint32_t cv[8];blake3_output o=blake3_chunk_output(&s->chunk);blake3_output_cv(&o,cv);blake3_add_chunk_cv(s,cv,s->chunk.chunk_counter+1);blake3_chunk_init(&s->chunk,s->key,s->chunk.chunk_counter+1,s->chunk.flags);} {size_t want=1024-blake3_chunk_len(&s->chunk),take=n<want?n:want;blake3_chunk_update(&s->chunk,in,take);in+=take;n-=take;}}}
static inline void blake3_hasher_finalize_seek(const blake3_hasher*s,uint64_t seek,uint8_t*out,size_t n){blake3_output o=blake3_chunk_output(&s->chunk);uint8_t parents=s->cv_stack_len;while(parents){uint32_t left[8],right[8];blake3_output_cv(&o,right);parents--;memcpy(left,s->cv_stack+(size_t)parents*32,32);o=blake3_parent_output(left,right,s->key,s->chunk.flags);}blake3_root_bytes(&o,seek,out,n);}
static inline void blake3_hasher_finalize(const blake3_hasher*s,uint8_t*out,size_t n){blake3_hasher_finalize_seek(s,0,out,n);}
static inline void blake3_hasher_init_derive_key(blake3_hasher*s,const char*context){uint8_t key[32];blake3_hasher_init_internal(s,blake3_iv,BLAKE3_DERIVE_KEY_CONTEXT);blake3_hasher_update(s,context,strlen(context));blake3_hasher_finalize(s,key,32);blake3_hasher_init_keyed(s,key);s->chunk.flags=BLAKE3_DERIVE_KEY_MATERIAL;}
#ifdef __cplusplus
}
#endif
#endif
