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

    mpz_t data; // mpz_t 类型的成员

    // 构造函数
    MyData() {
        mpz_init(data); // 初始化 mpz_t 成员
    }

    // 析构函数
    ~MyData() {
        mpz_clear(data); // 清理 mpz_t 成员
    }

    // 将 mpz_t 类型的整数转换为字符串*数字*，并返回结果
    string ReadData_string_number(unsigned int base = 36){
        char* str = mpz_get_str(NULL, base, data);
        std::string result(str);
        free(str); // 释放 mpz_get_str 分配的内存
        return result;
    }

    // 将 mpz_t 类型的整数转换为字符串，并返回结果
    string ReadData_string(unsigned int base = 36) {

        mpz_t tempData;
        mpz_init_set(tempData, data); // 初始化 tempData 并设置为 data 的副本

        // 获取 data 的位数
        size_t numBytes = mpz_sizeinbase(tempData, 256);
        std::string result(numBytes, '\0'); // 初始化结果字符串

        // 从 tempData 中提取每个字节的值
        for (size_t i = 0; i < numBytes; ++i) {
            unsigned char byteValue = mpz_fdiv_ui(tempData, 256); // 获取当前字节的值
            result[numBytes - i - 1] = static_cast<char>(byteValue); // 存储到结果字符串中
            mpz_tdiv_q_ui(tempData, tempData, 256); // 更新 tempData，移除已提取的字节
        }

        // 清理临时变量
        mpz_clear(tempData);
        
        return result;
    }

    // 将字符串*数字*转换为 mpz_t 类型的整数，并赋值给 data 成员
    void WriteData_string_number(const std::string& str, string identification = " NONE ", unsigned int base = 36) {
        if (mpz_set_str(data, str.c_str(), base) == -1) {
            std::cerr << "字符串转换错误。标识：" << identification << std::endl;
        }
    }

    // 将字符串转换为 mpz_t 类型的整数，并赋值给 data 成员
    void WriteData_string(const std::string& str, string identification = " NONE ", unsigned int base = 36) {
        mpz_t byteInteger;
        mpz_init(byteInteger); // 初始化 mpz_t 变量

        // 将字符串转换为整数
        for (size_t i = 0; i < str.size(); ++i) {
            mpz_mul_ui(data, data, 256); // 将之前的结果左移 8 位
            mpz_add_ui(data, data, static_cast<unsigned char>(str[i])); // 加上当前字符的 ASCII 值
        }

        // 清理临时变量
        mpz_clear(byteInteger);
    }
};
