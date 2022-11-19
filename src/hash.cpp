//
// Created by yuki on 22-7-27.
//
#include "cstring"
#include <iostream>

const u_int16_t P = 131;
const u_int64_t mod = 121347933274582139;//1e17
//  212370440130137957
u_int64_t fx(u_int64_t a, u_int16_t b){
    u_int64_t ans = 0;
    while(b > 0){
        if(b & 1)ans = (ans + a)%mod;
        a = 2 * a % mod, b >>= 1;
    }
    return ans;
}

int get_len(const char * s, int max){
    for(int i = 0; i < max; i++){
        if(s[i] == 0)return i;
    }
    return max;
}

u_int64_t  get_hash(const char * s)	{
    u_int64_t h = *(u_int64_t *)s % mod;
    for(int i = 8; i < 128; i += 8){
        h = (h * P % mod+ (*(u_int64_t *)(s + i)) % mod) % mod;
    }
    return h;
}

