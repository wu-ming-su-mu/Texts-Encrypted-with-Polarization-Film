#pragma once
//<>
#include <iostream>
#include <gmp.h>
#include <vector>
#include <fstream>

//""
#include "struct.h"
#include "encrypt.h"
#include "compress.h"
#include "file.h"

using namespace std;

//mpz_t写入文件
void mpz_to_binary(const mpz_t& data, const std::string& filename) {
	// 打开文件以进行二进制写入
	std::ofstream outFile(filename, std::ios::out | std::ios::binary);
	if (!outFile.is_open()) {
		throw std::runtime_error("无法打开文件进行写入");
	}

	// 获取 data 的大小（以字节为单位）
	size_t size = (mpz_sizeinbase(data, 2) + CHAR_BIT - 1) / CHAR_BIT;
	char* buffer = new char[size];

	// 将 mpz_t 数据导出到缓冲区
	mpz_export(buffer, &size, 1, sizeof(buffer[0]), 0, 0, data);

	// 写入文件
	outFile.write(buffer, size);

	// 清理资源
	delete[] buffer;
	outFile.close();
}

//从文件读取mpz_t
void binary_to_mpz(const std::string& filename, mpz_t& data) {
	// 打开文件以进行二进制读取
	std::ifstream inFile(filename, std::ios::in | std::ios::binary);
	if (!inFile.is_open()) {
		throw std::runtime_error("无法打开文件进行读取");
	}

	// 获取文件大小
	inFile.seekg(0, std::ios::end);
	size_t size = inFile.tellg();
	inFile.seekg(0, std::ios::beg);

	// 读取文件内容到缓冲区
	char* buffer = new char[size];
	inFile.read(buffer, size);

	// 将缓冲区数据导入到 mpz_t
	mpz_import(data, size, 1, sizeof(buffer[0]), 0, 0, buffer);

	// 清理资源
	delete[] buffer;
	inFile.close();
}

// 将两个 mpz_t 格式的数写入文件
void writeMpzsToFile(const mpz_t& num1, const mpz_t& num2, const string& filename) {
	// 创建一个输出文件流
	ofstream outFile(filename);
	if (!outFile) {
		cerr << "无法打开文件" << endl;
		return;
	}

	// 将 mpz_t 对象转换为十进制字符串
	char* strNum1 = mpz_get_str(nullptr, 10, num1);
	char* strNum2 = mpz_get_str(nullptr, 10, num2);

	// 将十进制字符串写入文件
	outFile << strNum1 << endl;
	outFile << strNum2 << endl;

	// 释放内存
	free(strNum1);
	free(strNum2);

	// 关闭文件流
	outFile.close();

	cout << "数据已成功写入文件" << filename << endl;
}

// 从文件中读取两个 mpz_t 格式的数
void readMpzsFromFile(mpz_t& num1, mpz_t& num2, const string& filename) {
	ifstream inFile(filename);
	if (!inFile) {
		cerr << "无法打开文件" << endl;
		return;
	}

	string strNum1, strNum2;
	if (!(inFile >> strNum1 >> strNum2)) {
		cerr << "读取文件错误" << endl;
		inFile.close();
		return;
	}

	// 将十进制字符串转换为 mpz_t 对象
	mpz_set_str(num1, strNum1.c_str(), 10);
	mpz_set_str(num2, strNum2.c_str(), 10);

	inFile.close();

	cout << "数据已成功读取" << endl;
}

// 生成密钥并分别保存
void SaveKeys(string key_name) {
	// 生成
	mpz_t n, e, d;
	mpz_inits(n, e, d, NULL);
	keygen(n, e, d);

	// 保存
	writeMpzsToFile(n, e, key_name + ".PublicKey");
	writeMpzsToFile(n, d, key_name + ".PrivateKey");

	mpz_clears(n, e, d, NULL);
	cout << "保存成功\n";
}

// 存储MyData数组进入文件
void write_to_file(MyData* data, unsigned int number, string filename) {
	std::ofstream outFile(filename); 
	if (!outFile.is_open()) {
		std::cerr << "无法打开文件进行写入" << std::endl;
		return;
	}

	outFile << number << endl;
	// 写入数据到文件
	for (unsigned int i = 0; i < number; i++) {
		outFile << data[i].ReadData_string_number(36) << std::endl;
	}

	// 关闭文件
	outFile.close();
}

// 从文件中读取
unsigned int read_from_file(MyData*& data, string filename) {
	std::ifstream inFile(filename); // 打开文件用于读取
	if (!inFile.is_open()) {
		std::cerr << "无法打开文件进行读取" << std::endl;
		return 0;
	}

	unsigned int number;
	if (!(inFile >> number)) {
		std::cerr << "读取数量失败" << std::endl;
		return 0;
	}

	try {
		data = new MyData[number];
		for (unsigned int i = 0; i < number; i++) {
			std::string line;
			if (!(inFile >> line)) {
				throw std::runtime_error("读取数据失败");
			}
			data[i].WriteData_string_number(line, " input ", 36);
			//cout << "line:" << line << endl;
		}
	}
	catch (const std::exception& e) {
		std::cerr << "异常: " << e.what() << std::endl;
		delete[] data; // 释放已分配的内存
		data = nullptr; // 避免野指针
		return 0;
	}

	inFile.close(); // 关闭文件
	return number;
}