# FNV-Analysis
FNV Mix Algorithm Analysis for TEthashV1

In case of FNV (Fowler-Noll-Vo) Hash Algorithm is used on simple and fast hashing of short messages.
You can refer original FNV Hash algorithm wiki [here](https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function#FNV-1_hash)

### Where FNV Applied on ETHASH

- In [ETHASH](https://github.com/ethereum/wiki/wiki/Ethash) , FNV Hash is used on
  * 1) On Data aggregation function, MIX parts
       Current Applied FNV0 Hash Implementation is depricated now.[FNV-0_hash (deprecated)](https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function#FNV-0_hash_(deprecated))
       It is simple way of hashing algorithm
  ```
   hash = 0
   for each byte_of_data to be hashed
   	hash = hash × FNV_prime
   	hash = hash XOR octet_of_data
   return hash
  ```
  When Analysed FNV-0 , there's weak [avalanche effect](https://simple.wikipedia.org/wiki/Avalanche_effect) , when hash input changes on 1~2bits.
  So, It may be deprecated.
  We need to research newer FNV Hash or short message hash algorithm.

### TEThashV1 (Trust ETHASH Version1) has adapted FNV1A hash function

Recent days, Prevent ASIC Miners.
There's several proposals for change ETHASH Algorithms.
In April on this year, trustfarm also suggested to change Keccack algorithm and MIX algorithm. [Issue Suggestion](https://github.com/ethereum/EIPs/issues/958#issuecomment-377849594)

Other's also suggest to use FNV1 or FNV1A hash algorithm on MIX parts.
[EIP-969](https://github.com/ethereum/EIPs/blob/master/EIPS/eip-969.md) ,  [EIP-1057 - ProgPoW](https://github.com/ethereum/EIPs/blob/master/EIPS/eip-1057.md) , [EIP-1355](https://github.com/ethereum/EIPs/blob/master/EIPS/eip-1355.md)

There's implementation doesn't looks like FNV1A , Implementation Form is looks like FNV1.

In view of Original Algorithm FNV1A 
```
use hash offset
FNV-1a hash
The FNV-1a hash differs from the FNV-1 hash by only the order in which the multiply and XOR is performed:[8][10]

   hash = FNV_offset_basis
   for each byte_of_data to be hashed
   	hash = hash XOR byte_of_data
   	hash = hash × FNV_prime
   return hash
```
Big Differences are **FNV1A vs FNV1** is use of initial **FNV_offset_basis**.
Offset Makes one more computation, multiplication loop, and more secure hash effects than FNV1.

In TETHashV1, Adapts fully follow the FNV1A implementation.

### TETHASHV1 FNV1A implementation

Followings are reference implementation of FNV1A adapted in TETHashV1.

```
-- reference OpenCL implementation

#define __TETHASHV1__
#undef __ETHASH__

#define FNV_PRIME 0x01000193U
#define FNV_OFFSET_BASIS  0x811c9dc5U

#ifdef __ETHASH__
#define fnv(x, y)        ((x) * FNV_PRIME ^ (y))
#else  // default __TETHASHV1__ 
#define fnv1a(x, y)         ((((FNV_OFFSET_BASIS^(x))*FNV_PRIME) ^ (y)) * FNV_PRIME)
#endif
```
There's another implementation of Character Size (1Byte) hash message based FNV1A, It's called us , **FNV1c**
It's more secure compared with above hash implementation. But, FNV1A is also enough, when `fnv_reduce()`.
Finally, We consider realistic implementation and computing speeds, choosed **FNV1A**.

Here's TAO's implementation of FNV1c , 4times more computation, mulitplication loops.
```
#define FNV_PRIME   0x01000193U
#define FNV_OFFSET_BASIS  0x811c9dc5U

#define fnv1i(x)           (  (( (( ((                                          \
                                ( ((FNV_OFFSET_BASIS)^( ((x)>>24)&0x000000ff )) * FNV_PRIME) \
                            ^   (((x)>>16  )&0x000000ff)) * FNV_PRIME) \
                            ^   (((x)>>8   )&0x000000ff)) * FNV_PRIME) \
                            ^   (((x)      )&0x000000ff)) * FNV_PRIME) \
                            )
#define fnv1c(x, y)            ((fnv1i(x) ^ (y)) * FNV_PRIME)
```

### How to test and analysis reference test code.

You can compile it with simple in terminal.
No additional library needs, 

```
gcc -o fnvtest fnvcltest.c
```

And You can execute it
```
fnvtest

F(00,00)::VEC(0, 0, ffffffff, 0)::      FNV  :00000000, DF=00000000(00) DS(00000000), FNV1 :00000000, DF=00000000(00) DS(00000000), FNV1a:117697cd, DF=117697cd(17) DS(117697cd), FNV1c:1210d00f, DF=127f8dbf(20) DS(11a1725f),         F___RC=efe1b9c4, DF:efe1b9c4(19) , F1__RC=deb68dfe, DF:deb68dfe(22) , F1A_RC=99bad28b, DF:99bad28b(17) , F1C_RC=e29fa497, DF:e29fa497(18)
F(00,01)::VEC(0, 1, ffffffff, 0)::      FNV  :00000001, DF=00000001(01) DS(00000001), FNV1 :01000193, DF=01000193(06) DS(01000193), FNV1a:1076963a, DF=010001f7(09) DS(01000193), FNV1c:1110ce7c, DF=03001e73(11) DS(01000193),         F___RC=fefffe6d, DF:111e47a9(14) , F1__RC=d9fd8597, DF:074b0869(12) , F1A_RC=72c287e0, DF:eb78556b(19) , F1C_RC=6b6991ef, DF:89f63578(17)
```
`F(00,01)` : is input x,y 
`VEC(0, 1, ffffffff, 0)`  : is `fnv_reduce` input vector (a,b,c,d)
`FNV  :00000001, DF=00000001(01) DS(00000001)` : 
  * `FNV(00,01)` result is 00000001 , 
  * `DF` : is changed bitcounts, compared with previous outputs, in this case prev[00,00] current[00,01] input is 1bit changed, and output result 1bit changed.
  * `DS` : is difference of previous result and current result , ABS(prev_fnvresult,current_fnvresult).

** Basically, `DF` is higher is best on hash algorithm.

`F___RC=fefffe6d, DF:111e47a9(14)` : `fnv_reduce = fnv(fnv(fnv(a,b),c),d) ` result is fefffe6d , and Different Bits counts are `14` bits.

