# FNV-Analysis
FNV Mix Algorithm Analysis for TEthashV1

In case of FNV (Fowler-Noll-Vo) Hash Algorithm is used on simple and fast hashing of short messages.
You can refer original FNV Hash algorithm wiki [here](https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function#FNV-1_hash)

### Where FNV Applied on ETHASH

- In [ETHASH](https://github.com/ethereum/wiki/wiki/Ethash) , FNV Hash is used on
  * 1) On Data aggregation function, MIX parts.
  
       Current Applied FNV0 Hash Implementation is depricated now.
       [FNV-0_hash (deprecated)](https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function#FNV-0_hash_(deprecated))
       
       It is simple way of hashing algorithm
  ```
   hash = 0
   for each byte_of_data to be hashed
   	hash = hash × FNV_prime
   	hash = hash XOR octet_of_data
   return hash
  ```
  When Analysed FNV-0 , there's weak [avalanche effect](https://simple.wikipedia.org/wiki/Avalanche_effect), when hash input changes on 1~2bits.
  So, It may be deprecated.
  
  We need to research newer FNV Hash or short message hash algorithm.

### TEThashV1 (Trust ETHASH Version1) has adapted FNV1A hash function

Recent days, Prevent ASIC Miners.
There's several proposals for change ETHASH Algorithms.

In April on this year,

@trustfarm-dev also suggested to change Keccack algorithm and MIX algorithm. [Issue Suggestion](https://github.com/ethereum/EIPs/issues/958#issuecomment-377849594)

Other's also suggest to use FNV1 or FNV1A hash algorithm on MIX parts.

[EIP-969](https://github.com/ethereum/EIPs/blob/master/EIPS/eip-969.md) ,  
[EIP-1057 - ProgPoW](https://github.com/ethereum/EIPs/blob/master/EIPS/eip-1057.md) , 
[EIP-1355](https://github.com/ethereum/EIPs/blob/master/EIPS/eip-1355.md)

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
-- Reference Pseudo implementation

#define __TETHASHV1__
#undef __ETHASH__

#define FNV_PRIME 0x01000193U
#define FNV_OFFSET_BASIS  0x811c9dc5U

#ifdef __ETHASH__

#define fnv(x, y)        ((x) * FNV_PRIME ^ (y))
#define fnv_reduce(a,b,c,d) (fnv(fnv(fnv(a, b), c), d))

#else  // default __TETHASHV1__ 

#define fnv1a(x, y)         ((((FNV_OFFSET_BASIS^(x))*FNV_PRIME) ^ (y)) * FNV_PRIME)
#define fnv1a_reduce(a,b,c,d) (fnv1a(fnv1a(fnv1a(a, b), c), d))

#endif

```
There's another implementation of Character Size (1Byte) hash message based FNV1A, It's called us , **FNV1c**
It's more secure compared with above hash implementation. 

But, FNV1A is also enough, when `fnv_reduce()`.

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
F(00,02)::VEC(0, 2, ffffffff, 0)::      FNV  :00000002, DF=00000003(02) DS(00000001), FNV1 :02000326, DF=030002b5(08) DS(01000193), FNV1a:0f7694a7, DF=1f00029d(11) DS(01000193), FNV1c:1410d335, DF=05001d49(09) DS(030004b9),         F___RC=d8fd8404, DF:26027a69(13) , F1__RC=9b16d24c, DF:42eb57db(19) , F1A_RC=c17f0ecb, DF:b3bd892b(18) , F1C_RC=a5be8e78, DF:ced71f97(21)
F(00,03)::VEC(0, 3, ffffffff, 0)::      FNV  :00000003, DF=00000001(01) DS(00000001), FNV1 :030004b9, DF=0100079f(10) DS(01000193), FNV1a:0e769314, DF=010007b3(09) DS(01000193), FNV1c:1310d1a2, DF=07000297(09) DS(01000193),         F___RC=b2fb099b, DF:6a068d9f(16) , F1__RC=5c301f01, DF:c726cd4d(17) , F1A_RC=94cf402e, DF:55b04ee5(16) , F1C_RC=aea1a025, DF:0b1f2e5d(17)
F(00,04)::VEC(0, 4, ffffffff, 0)::      FNV  :00000004, DF=00000007(03) DS(00000001), FNV1 :0400064c, DF=070002f5(10) DS(01000193), FNV1a:0d769181, DF=03000295(07) DS(01000193), FNV1c:0e10c9c3, DF=1d001861(09) DS(050007df),         F___RC=8cf88f32, DF:3e0386a9(14) , F1__RC=1d496bb6, DF:417974b7(17) , F1A_RC=89401d59, DF:1d8f5d77(20) , F1C_RC=e4e96c7c, DF:4a48cc59(13)
F(00,05)::VEC(0, 5, ffffffff, 0)::      FNV  :00000005, DF=00000001(01) DS(00000001), FNV1 :050007df, DF=01000193(06) DS(01000193), FNV1a:0c768fee, DF=01001e6f(11) DS(01000193), FNV1c:0d10c830, DF=030001f3(09) DS(01000193),         F___RC=66f614c9, DF:ea0e9bfb(20) , F1__RC=de62b86b, DF:c32bd3dd(19) , F1A_RC=346e222c, DF:bd2e3f75(21) , F1C_RC=502e5f82, DF:b4c733fe(20)
F(00,06)::VEC(0, 6, ffffffff, 0)::      FNV  :00000006, DF=00000003(02) DS(00000001), FNV1 :06000972, DF=03000ead(10) DS(01000193), FNV1a:0b768e5b, DF=070001b5(09) DS(01000193), FNV1c:1010cce9, DF=1d0004d9(10) DS(030004b9),         F___RC=40f39a60, DF:26058ea9(13) , F1__RC=9f7c0520, DF:411ebd4b(16) , F1A_RC=b376a527, DF:8718870b(13) , F1C_RC=1241a9a4, DF:426ff626(17)
F(00,07)::VEC(0, 7, ffffffff, 0)::      FNV  :00000007, DF=00000001(01) DS(00000001), FNV1 :07000b05, DF=01000277(08) DS(01000193), FNV1a:0a768cc8, DF=01000293(06) DS(01000193), FNV1c:0f10cb56, DF=1f0007bf(15) DS(01000193),         F___RC=1af11ff7, DF:5a028597(13) , F1__RC=609551d5, DF:ffe954f5(22) , F1A_RC=14293bea, DF:a75f9ecd(21) , F1C_RC=49d34bba, DF:5b92e21e(16)
F(00,08)::VEC(0, 8, ffffffff, 0)::      FNV  :00000008, DF=0000000f(04) DS(00000001), FNV1 :08000c98, DF=0f00079d(12) DS(01000193), FNV1a:09768b35, DF=030007fd(12) DS(01000193), FNV1c:1a10dca7, DF=150017f1(12) DS(0b001151),         F___RC=f4eea58e, DF:ee1fba79(21) , F1__RC=21ae9e8a, DF:413bcf5f(19) , F1A_RC=eeebb7a5, DF:fac28c4f(17) , F1C_RC=7da04f47, DF:347304fd(16)
F(00,09)::VEC(0, 9, ffffffff, 0)::      FNV  :00000009, DF=00000001(01) DS(00000001), FNV1 :09000e2b, DF=010002b3(07) DS(01000193), FNV1a:087689a2, DF=01000297(07) DS(01000193), FNV1c:1910db14, DF=030007b3(10) DS(01000193),         F___RC=ceec2b25, DF:3a028eab(14) , F1__RC=e2c7eb3f, DF:c36975b5(18) , F1A_RC=54e1aef8, DF:ba0a195d(15) , F1C_RC=d425e1af, DF:a985aee8(16)
F(00,0a)::VEC(0, a, ffffffff, 0)::      FNV  :0000000a, DF=00000003(02) DS(00000001), FNV1 :0a000fbe, DF=03000195(07) DS(01000193), FNV1a:0776880f, DF=0f0001ad(10) DS(01000193), FNV1c:1c10dfcd, DF=050004d9(08) DS(030004b9),         F___RC=a8e9b0bc, DF:66059b99(15) , F1__RC=a3e137f4, DF:4126dccb(15) , F1A_RC=213fcd63, DF:75de639b(20) , F1C_RC=7e1d2751, DF:aa38c6fe(18)
F(00,0b)::VEC(0, b, ffffffff, 0)::      FNV  :0000000b, DF=00000001(01) DS(00000001), FNV1 :0b001151, DF=01001eef(12) DS(01000193), FNV1a:0676867c, DF=01000e73(09) DS(01000193), FNV1c:1b10de3a, DF=070001f7(11) DS(01000193),         F___RC=82e73653, DF:2a0e86ef(16) , F1__RC=64fa84a9, DF:c71bb35d(19) , F1A_RC=5598ce46, DF:74a70325(14) , F1C_RC=6400c630, DF:1a1de161(14)
F(00,0c)::VEC(0, c, ffffffff, 0)::      FNV  :0000000c, DF=00000007(03) DS(00000001), FNV1 :0c0012e4, DF=070003b5(10) DS(01000193), FNV1a:057684e9, DF=03000295(07) DS(01000193), FNV1c:1610d65b, DF=0d000861(07) DS(050007df),         F___RC=5ce4bbea, DF:de038db9(17) , F1__RC=2613d15e, DF:42e955f7(18) , F1A_RC=6a220ff1, DF:3fbac1b7(20) , F1C_RC=6e781da4, DF:0a78db94(15)
F(00,0d)::VEC(0, d, ffffffff, 0)::      FNV  :0000000d, DF=00000001(01) DS(00000001), FNV1 :0d001477, DF=01000693(07) DS(01000193), FNV1a:04768356, DF=010007bf(11) DS(01000193), FNV1c:1510d4c8, DF=03000293(07) DS(01000193),         F___RC=36e24181, DF:6a06fa6b(17) , F1__RC=e72d1e13, DF:c13ecf4d(18) , F1A_RC=168d4944, DF:7caf46b5(19) , F1C_RC=65bbcfa1, DF:0bc3d205(13)
F(00,0e)::VEC(0, e, ffffffff, 0)::      FNV  :0000000e, DF=00000003(02) DS(00000001), FNV1 :0e00160a, DF=0300027d(09) DS(01000193), FNV1a:037681c3, DF=07000295(08) DS(01000193), FNV1c:1810d981, DF=0d000d49(09) DS(030004b9),         F___RC=10dfc718, DF:263d8699(15) , F1__RC=a8466ac8, DF:4f6b74db(20) , F1A_RC=93e667bf, DF:856b2efb(19) , F1C_RC=76f80ee3, DF:1343c142(11)
F(00,0f)::VEC(0, f, ffffffff, 0)::      FNV  :0000000f, DF=00000001(01) DS(00000001), FNV1 :0f00179d, DF=01000197(07) DS(01000193), FNV1a:02768030, DF=010001f3(08) DS(01000193), FNV1c:1710d7ee, DF=0f000e6f(13) DS(01000193),         F___RC=eadd4caf, DF:fa028bb7(17) , F1__RC=695fb77d, DF:c119ddb5(17) , F1A_RC=0f485682, DF:9cae313d(17) , F1C_RC=3667e8dc, DF:409fe63f(18)
F(00,10)::VEC(0, 10, ffffffff, 0)::     FNV  :00000010, DF=0000001f(05) DS(00000001), FNV1 :10001930, DF=1f000ead(13) DS(01000193), FNV1a:01767e9d, DF=0300fead(14) DS(01000193), FNV1c:0210b6df, DF=15006131(09) DS(1500210f),         F___RC=c4dad246, DF:2e079ee9(17) , F1__RC=2a790432, DF:4326b34f(16) , F1A_RC=d10adebd, DF:de42883f(16) , F1C_RC=1ce48e12, DF:2a8366ce(15)
```

`F(00,01)` : is input x,y 

`VEC(0, 1, ffffffff, 0)`  : is `fnv_reduce` input vector (a,b,c,d)

`FNV  :00000001, DF=00000001(01) DS(00000001)` : 
  * `FNV(00,01)` result is 00000001 , 
  * `DF` : is changed bitcounts, compared with previous outputs, in this case prev[00,00] current[00,01] input is 1bit changed, and output result 1bit changed.
  * `DS` : is difference of previous result and current result , ABS(prev_fnvresult,current_fnvresult).

** Basically, `DF` is higher is best on hash algorithm.

`F___RC=fefffe6d, DF:111e47a9(14)` : `fnv_reduce = fnv(fnv(fnv(a,b),c),d) ` result is fefffe6d , and Different Bits counts are `14` bits.

