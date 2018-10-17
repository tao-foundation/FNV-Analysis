#include <stdio.h>

/*
  https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function#FNV-1_hash

  FNV_hash0 is depricated
  use FNV1 hash

use hash offset
FNV-1a hash
The FNV-1a hash differs from the FNV-1 hash by only the order in which the multiply and XOR is performed:[8][10]

   hash = FNV_offset_basis
   for each byte_of_data to be hashed
   	hash = hash XOR byte_of_data
   	hash = hash × FNV_prime
   return hash

Size in bits
{\displaystyle n=2^{s}} {\displaystyle n=2^{s}}

FNV prime	FNV offset basis
32	224 + 28 + 0x93 = 16777619

2166136261 = 0x811c9dc5
*/

#define CRC32_PRIME 0x04C11DB5U     /* CRC32 (0x04C11DB7 - 2) for PrimeNumber */
#define FNV_PRIME   0x01000193U
#define FNV_OFFSET_BASIS  0x811c9dc5U

#define fnv1i(x)           (  (( (( ((                                          \
                                ( ((FNV_OFFSET_BASIS)^( ((x)>>24)&0x000000ff )) * FNV_PRIME) \
                            ^   (((x)>>16  )&0x000000ff)) * FNV_PRIME) \
                            ^   (((x)>>8   )&0x000000ff)) * FNV_PRIME) \
                            ^   (((x)      )&0x000000ff)) * FNV_PRIME) \
                            )
#define fnv1c(x, y)            ((fnv1i(x) ^ (y)) * FNV_PRIME)

#define fnv1a(x, y)         ((((FNV_OFFSET_BASIS^(x))*FNV_PRIME) ^ (y)) * FNV_PRIME)

#define fnv1c_reduce(v) fnv1c(fnv1c(fnv1c(v.x, v.y), v.z), v.w)

#define fnv1a_reduce(v) fnv1a(fnv1a(fnv1a(v.x, v.y), v.z), v.w)

/*
#define fnv1iv(x)           (  (( (( ((                                          \
                                ( ((FNV_OFFSET_BASIS)^( ((x)>>24)&0x000000ff )) * FNV_PRIME) \
                            ^   (((x)>>16  )&0x000000ff)) * FNV_PRIME) \
                            ^   (((x)>>8   )&0x000000ff)) * FNV_PRIME) \
                            ^   (((x)      )&0x000000ff)) * FNV_PRIME) \
                            )
#define fnv1c(x, y)            (((fnv1iv(x) ^ (y)) * FNV_PRIME))
*/
#define fnv(x, y) ((x)*FNV_PRIME ^ (y))
#define fnv_reduce(v) fnv(fnv(fnv(v.x, v.y), v.z), v.w)
#define fnv1(x, y) (((x) ^ (y)) * FNV_PRIME)
#define fnv1_reduce(v) fnv1(fnv1(fnv1(v.x, v.y), v.z), v.w)

struct vector {
    unsigned int x,y,z,w;
};

#define ABS(x, y)  (((x)>(y))?((x)-(y)):((y)-(x)))

int nbits(unsigned int v);

int main(int argc, char *argv)
{
    unsigned int    x = 0, y = 0;
    unsigned int    fnv_out, fnv1_out, fnv1a_out, fnv1c_out;
    unsigned int    fnv_out_p = 0, fnv1_out_p = 0, fnv1a_out_p = 0, fnv1c_out_p;
    unsigned int    fnv_reduce, fnv1_reduce, fnv1a_reduce, fnv1c_reduce;
    unsigned int    fnv_reduce_p = 0, fnv1_reduce_p = 0, fnv1a_reduce_p = 0, fnv1c_reduce_p = 0;
    unsigned int    fnv_diff, fnv1_diff, fnv1a_diff, fnv1c_diff;
    unsigned int    fnv_dist, fnv1_dist, fnv1a_dist, fnv1c_dist;
    unsigned int    fnv_rdcdiff, fnv1_rdcdiff, fnv1a_rdcdiff, fnv1c_rdcdiff;
    struct vector v = {1,2,3,4};

    for (int i = 0 ; i < 0x1000 ; i ++)
    {
        x = i;
        for (int j = 0; j < 0x100 ; j ++)
        {
            y = j;

            fnv_out = fnv(x, y);
            fnv_reduce = fnv_reduce(v);

            fnv1_out = fnv1(x, y);
            fnv1_reduce = fnv1_reduce(v);

            fnv1a_out = fnv1a(x, y);
            fnv1a_reduce = fnv1a_reduce(v);

            fnv1c_out = fnv1c(x, y);
            fnv1c_reduce = fnv1c_reduce(v);


            v.x = i; v.y = j; v.z = 0xFFFFFFFF-i ; v.w = i;

            fnv_diff = fnv_out ^ fnv_out_p;
            fnv1_diff = fnv1_out ^ fnv1_out_p;
            fnv1a_diff = fnv1a_out ^ fnv1a_out_p;
            fnv1c_diff = fnv1c_out ^ fnv1c_out_p;

            fnv_dist = ABS(fnv_out ,fnv_out_p);
            fnv1_dist = ABS(fnv1_out ,fnv1_out_p);
            fnv1a_dist = ABS(fnv1a_out , fnv1a_out_p);
            fnv1c_dist = ABS(fnv1c_out , fnv1c_out_p);

            fnv_rdcdiff = fnv_reduce ^ fnv_reduce_p;
            fnv1_rdcdiff = fnv1_reduce ^ fnv1_reduce_p;
            fnv1a_rdcdiff = fnv1a_reduce ^ fnv1a_reduce_p;
            fnv1c_rdcdiff = fnv1c_reduce ^ fnv1c_reduce_p;

            printf(
                "F(%02x,%02x)::VEC(%x, %x, %x, %x):: "
                "\t"
                "FNV  :%08x, DF=%08x(%02d) DS(%08x), "
                "FNV1 :%08x, DF=%08x(%02d) DS(%08x), "
                "FNV1a:%08x, DF=%08x(%02d) DS(%08x), "
                "FNV1c:%08x, DF=%08x(%02d) DS(%08x), "
                "\t"
                "F___RC=%08x, DF:%08x(%2d) , "
                "F1__RC=%08x, DF:%08x(%2d) , "
                "F1A_RC=%08x, DF:%08x(%2d) , "
                "F1C_RC=%08x, DF:%08x(%2d) "
                "\n",
                x, y, v.x, v.y, v.z, v.w, 
                fnv_out, fnv_diff, nbits(fnv_diff), fnv_dist, 
                fnv1_out, fnv1_diff, nbits(fnv1_diff), fnv1_dist, 
                fnv1a_out, fnv1a_diff, nbits(fnv1a_diff), fnv1a_dist, 
                fnv1c_out, fnv1c_diff, nbits(fnv1c_diff), fnv1c_dist, 
                fnv_reduce, fnv_rdcdiff, nbits(fnv_rdcdiff), 
                fnv1_reduce, fnv1_rdcdiff, nbits(fnv1_rdcdiff),
                fnv1a_reduce, fnv1a_rdcdiff, nbits(fnv1a_rdcdiff), 
                fnv1c_reduce, fnv1c_rdcdiff, nbits(fnv1c_rdcdiff)
                );

            fnv_out_p = fnv_out;
            fnv1_out_p = fnv1_out;
            fnv1a_out_p = fnv1a_out;
            fnv1c_out_p = fnv1c_out;
            fnv_reduce_p = fnv_reduce;
            fnv1_reduce_p = fnv1_reduce;
            fnv1a_reduce_p = fnv1a_reduce;
            fnv1c_reduce_p = fnv1c_reduce;
        }
    }

    return 0;
}

int nbits(unsigned int v)
{
    unsigned int c; // c accumulates the total bits set in v

    for (c = 0; v; v >>= 1)
    {
       c += v & 1;
    }
    return c;
}
