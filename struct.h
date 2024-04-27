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

struct MyData {

    mpz_t data; // mpz_t ���͵ĳ�Ա

    // ���캯��
    MyData() {
        mpz_init(data); // ��ʼ�� mpz_t ��Ա
    }

    // ��������
    ~MyData() {
        mpz_clear(data); // ���� mpz_t ��Ա
    }

    // �� mpz_t ���͵�����ת��Ϊ�ַ���*����*�������ؽ��
    string ReadData_string_number(unsigned int base = 36){
        char* str = mpz_get_str(NULL, base, data);
        std::string result(str);
        free(str); // �ͷ� mpz_get_str ������ڴ�
        return result;
    }

    // �� mpz_t ���͵�����ת��Ϊ�ַ����������ؽ��
    string ReadData_string(unsigned int base = 36) {

        mpz_t tempData;
        mpz_init_set(tempData, data); // ��ʼ�� tempData ������Ϊ data �ĸ���

        // ��ȡ data ��λ��
        size_t numBytes = mpz_sizeinbase(tempData, 256);
        std::string result(numBytes, '\0'); // ��ʼ������ַ���

        // �� tempData ����ȡÿ���ֽڵ�ֵ
        for (size_t i = 0; i < numBytes; ++i) {
            unsigned char byteValue = mpz_fdiv_ui(tempData, 256); // ��ȡ��ǰ�ֽڵ�ֵ
            result[numBytes - i - 1] = static_cast<char>(byteValue); // �洢������ַ�����
            mpz_tdiv_q_ui(tempData, tempData, 256); // ���� tempData���Ƴ�����ȡ���ֽ�
        }

        // ������ʱ����
        mpz_clear(tempData);
        
        return result;
    }

    // ���ַ���*����*ת��Ϊ mpz_t ���͵�����������ֵ�� data ��Ա
    void WriteData_string_number(const std::string& str, string identification = " NONE ", unsigned int base = 36) {
        if (mpz_set_str(data, str.c_str(), base) == -1) {
            std::cerr << "�ַ���ת�����󡣱�ʶ��" << identification << std::endl;
        }
    }

    // ���ַ���ת��Ϊ mpz_t ���͵�����������ֵ�� data ��Ա
    void WriteData_string(const std::string& str, string identification = " NONE ", unsigned int base = 36) {
        mpz_t byteInteger;
        mpz_init(byteInteger); // ��ʼ�� mpz_t ����

        // ���ַ���ת��Ϊ����
        for (size_t i = 0; i < str.size(); ++i) {
            mpz_mul_ui(data, data, 256); // ��֮ǰ�Ľ������ 8 λ
            mpz_add_ui(data, data, static_cast<unsigned char>(str[i])); // ���ϵ�ǰ�ַ��� ASCII ֵ
        }

        // ������ʱ����
        mpz_clear(byteInteger);
    }
};
