#pragma once
//<>
#include <iostream>
#include <gmp.h>
#include <fstream>

//""
#include "struct.h"
#include "encrypt.h"
#include "compress.h"

using namespace std;

// 一个函数用于检查一个数是否是素数
bool is_prime(mpz_t n) {
    return mpz_probab_prime_p(n, 25) > 0; // 使用Miller-Rabin素性测试，重复25次
}

// 一个函数用于在给定范围内生成一个随机素数
void random_prime(mpz_t p, mpz_t min, mpz_t max) {
    gmp_randstate_t state; // 随机数生成器的状态
    gmp_randinit_default(state); // 初始化随机数生成器
    gmp_randseed_ui(state, time(NULL)); // 用当前时间作为种子
    do {
        mpz_urandomm(p, state, max); // 生成一个[0, max)范围内的随机数
        mpz_add(p, p, min); // 加上min，得到[min, max)范围内的随机数
    } while (!is_prime(p)); // 检查是否是素数，如果不是，重复生成
    gmp_randclear(state); // 清除随机数生成器
}

// 一个函数用于计算两个数的最大公约数
void gcd(mpz_t g, mpz_t a, mpz_t b) {
    mpz_gcd(g, a, b); // 使用gmp提供的gcd函数
}

// 一个函数用于计算一个数的模逆元
void mod_inverse(mpz_t r, mpz_t a, mpz_t m) {
    mpz_invert(r, a, m); // 使用gmp提供的模逆元函数
}

// 一个函数用于计算一个数的模幂
void mod_pow(mpz_t r, mpz_t x, mpz_t y, mpz_t m) {
    mpz_powm(r, x, y, m); // 使用gmp提供的模幂函数
}

// 一个函数用于生成RSA的公钥和私钥对
void keygen(mpz_t n, mpz_t e, mpz_t d) {
    // 选择两个不同的素数p和q
    mpz_t p, q, min, max;
    mpz_inits(p, q, min, max, NULL); // 初始化变量
    mpz_ui_pow_ui(min, 2, 3072); // 设置min为2^3072
    mpz_ui_pow_ui(max, 2, 3073); // 设置max为2^3073
    random_prime(p, min, max); // 生成一个随机素数p
    random_prime(q, min, max); // 生成一个随机素数q
    while (mpz_cmp(p, q) == 0) random_prime(q, min, max); // 如果p和q相等，重新生成q

    // 计算n = p * q
    mpz_mul(n, p, q);

    // 计算phi(n) = (p - 1) * (q - 1)
    mpz_t phi;
    mpz_init(phi); // 初始化变量
    mpz_sub_ui(p, p, 1); // p = p - 1
    mpz_sub_ui(q, q, 1); // q = q - 1
    mpz_mul(phi, p, q); // phi = p * q

    // 选择一个满足1 < e < phi且gcd(e, phi) = 1的整数e
    mpz_set_str(e, "65537", 10); // 设置e为65537，这是一个常用的公钥指数
    mpz_t g;
    mpz_init(g); // 初始化变量
    gcd(g, e, phi); // 计算gcd(e, phi)
    while (mpz_cmp_ui(g, 1) != 0) { // 如果gcd(e, phi)不等于1
        mpz_add_ui(e, e, 2); // e = e + 2
        gcd(g, e, phi); // 重新计算gcd(e, phi)
    }

    // 计算d = e^(-1) mod phi
    mod_inverse(d, e, phi); // 使用gmp提供的模逆元函数

    // 清除变量
    mpz_clears(p, q, min, max, phi, g, NULL);
}

// 一个函数用于使用RSA加密一条消息
void encrypt(mpz_t result, mpz_t message, mpz_t n, mpz_t e) {
    // 计算c = m^e mod n
    mod_pow(result, message, e, n); // 使用gmp提供的模幂函数
}

// 一个函数用于使用RSA解密一条消息
void decrypt(mpz_t result, mpz_t message, mpz_t n, mpz_t d) {
    // 计算m = c^d mod n
    mod_pow(result, message, d, n); // 使用gmp提供的模幂函数
}
