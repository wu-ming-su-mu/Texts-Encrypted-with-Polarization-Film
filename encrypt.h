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

// һ���������ڼ��һ�����Ƿ�������
bool is_prime(mpz_t n) {
    return mpz_probab_prime_p(n, 25) > 0; // ʹ��Miller-Rabin���Բ��ԣ��ظ�25��
}

// һ�����������ڸ�����Χ������һ���������
void random_prime(mpz_t p, mpz_t min, mpz_t max) {
    gmp_randstate_t state; // �������������״̬
    gmp_randinit_default(state); // ��ʼ�������������
    gmp_randseed_ui(state, time(NULL)); // �õ�ǰʱ����Ϊ����
    do {
        mpz_urandomm(p, state, max); // ����һ��[0, max)��Χ�ڵ������
        mpz_add(p, p, min); // ����min���õ�[min, max)��Χ�ڵ������
    } while (!is_prime(p)); // ����Ƿ���������������ǣ��ظ�����
    gmp_randclear(state); // ��������������
}

// һ���������ڼ��������������Լ��
void gcd(mpz_t g, mpz_t a, mpz_t b) {
    mpz_gcd(g, a, b); // ʹ��gmp�ṩ��gcd����
}

// һ���������ڼ���һ������ģ��Ԫ
void mod_inverse(mpz_t r, mpz_t a, mpz_t m) {
    mpz_invert(r, a, m); // ʹ��gmp�ṩ��ģ��Ԫ����
}

// һ���������ڼ���һ������ģ��
void mod_pow(mpz_t r, mpz_t x, mpz_t y, mpz_t m) {
    mpz_powm(r, x, y, m); // ʹ��gmp�ṩ��ģ�ݺ���
}

// һ��������������RSA�Ĺ�Կ��˽Կ��
void keygen(mpz_t n, mpz_t e, mpz_t d) {
    // ѡ��������ͬ������p��q
    mpz_t p, q, min, max;
    mpz_inits(p, q, min, max, NULL); // ��ʼ������
    mpz_ui_pow_ui(min, 2, 3072); // ����minΪ2^3072
    mpz_ui_pow_ui(max, 2, 3073); // ����maxΪ2^3073
    random_prime(p, min, max); // ����һ���������p
    random_prime(q, min, max); // ����һ���������q
    while (mpz_cmp(p, q) == 0) random_prime(q, min, max); // ���p��q��ȣ���������q

    // ����n = p * q
    mpz_mul(n, p, q);

    // ����phi(n) = (p - 1) * (q - 1)
    mpz_t phi;
    mpz_init(phi); // ��ʼ������
    mpz_sub_ui(p, p, 1); // p = p - 1
    mpz_sub_ui(q, q, 1); // q = q - 1
    mpz_mul(phi, p, q); // phi = p * q

    // ѡ��һ������1 < e < phi��gcd(e, phi) = 1������e
    mpz_set_str(e, "65537", 10); // ����eΪ65537������һ�����õĹ�Կָ��
    mpz_t g;
    mpz_init(g); // ��ʼ������
    gcd(g, e, phi); // ����gcd(e, phi)
    while (mpz_cmp_ui(g, 1) != 0) { // ���gcd(e, phi)������1
        mpz_add_ui(e, e, 2); // e = e + 2
        gcd(g, e, phi); // ���¼���gcd(e, phi)
    }

    // ����d = e^(-1) mod phi
    mod_inverse(d, e, phi); // ʹ��gmp�ṩ��ģ��Ԫ����

    // �������
    mpz_clears(p, q, min, max, phi, g, NULL);
}

// һ����������ʹ��RSA����һ����Ϣ
void encrypt(mpz_t result, mpz_t message, mpz_t n, mpz_t e) {
    // ����c = m^e mod n
    mod_pow(result, message, e, n); // ʹ��gmp�ṩ��ģ�ݺ���
}

// һ����������ʹ��RSA����һ����Ϣ
void decrypt(mpz_t result, mpz_t message, mpz_t n, mpz_t d) {
    // ����m = c^d mod n
    mod_pow(result, message, d, n); // ʹ��gmp�ṩ��ģ�ݺ���
}
