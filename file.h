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

//mpz_tд���ļ�
void mpz_to_binary(const mpz_t& data, const std::string& filename) {
	// ���ļ��Խ��ж�����д��
	std::ofstream outFile(filename, std::ios::out | std::ios::binary);
	if (!outFile.is_open()) {
		throw std::runtime_error("�޷����ļ�����д��");
	}

	// ��ȡ data �Ĵ�С�����ֽ�Ϊ��λ��
	size_t size = (mpz_sizeinbase(data, 2) + CHAR_BIT - 1) / CHAR_BIT;
	char* buffer = new char[size];

	// �� mpz_t ���ݵ�����������
	mpz_export(buffer, &size, 1, sizeof(buffer[0]), 0, 0, data);

	// д���ļ�
	outFile.write(buffer, size);

	// ������Դ
	delete[] buffer;
	outFile.close();
}

//���ļ���ȡmpz_t
void binary_to_mpz(const std::string& filename, mpz_t& data) {
	// ���ļ��Խ��ж����ƶ�ȡ
	std::ifstream inFile(filename, std::ios::in | std::ios::binary);
	if (!inFile.is_open()) {
		throw std::runtime_error("�޷����ļ����ж�ȡ");
	}

	// ��ȡ�ļ���С
	inFile.seekg(0, std::ios::end);
	size_t size = inFile.tellg();
	inFile.seekg(0, std::ios::beg);

	// ��ȡ�ļ����ݵ�������
	char* buffer = new char[size];
	inFile.read(buffer, size);

	// �����������ݵ��뵽 mpz_t
	mpz_import(data, size, 1, sizeof(buffer[0]), 0, 0, buffer);

	// ������Դ
	delete[] buffer;
	inFile.close();
}

// ������ mpz_t ��ʽ����д���ļ�
void writeMpzsToFile(const mpz_t& num1, const mpz_t& num2, const string& filename) {
	// ����һ������ļ���
	ofstream outFile(filename);
	if (!outFile) {
		cerr << "�޷����ļ�" << endl;
		return;
	}

	// �� mpz_t ����ת��Ϊʮ�����ַ���
	char* strNum1 = mpz_get_str(nullptr, 10, num1);
	char* strNum2 = mpz_get_str(nullptr, 10, num2);

	// ��ʮ�����ַ���д���ļ�
	outFile << strNum1 << endl;
	outFile << strNum2 << endl;

	// �ͷ��ڴ�
	free(strNum1);
	free(strNum2);

	// �ر��ļ���
	outFile.close();

	cout << "�����ѳɹ�д���ļ�" << filename << endl;
}

// ���ļ��ж�ȡ���� mpz_t ��ʽ����
void readMpzsFromFile(mpz_t& num1, mpz_t& num2, const string& filename) {
	ifstream inFile(filename);
	if (!inFile) {
		cerr << "�޷����ļ�" << endl;
		return;
	}

	string strNum1, strNum2;
	if (!(inFile >> strNum1 >> strNum2)) {
		cerr << "��ȡ�ļ�����" << endl;
		inFile.close();
		return;
	}

	// ��ʮ�����ַ���ת��Ϊ mpz_t ����
	mpz_set_str(num1, strNum1.c_str(), 10);
	mpz_set_str(num2, strNum2.c_str(), 10);

	inFile.close();

	cout << "�����ѳɹ���ȡ" << endl;
}

// ������Կ���ֱ𱣴�
void SaveKeys(string key_name) {
	// ����
	mpz_t n, e, d;
	mpz_inits(n, e, d, NULL);
	keygen(n, e, d);

	// ����
	writeMpzsToFile(n, e, key_name + ".PublicKey");
	writeMpzsToFile(n, d, key_name + ".PrivateKey");

	mpz_clears(n, e, d, NULL);
	cout << "����ɹ�\n";
}

// �洢MyData��������ļ�
void write_to_file(MyData* data, unsigned int number, string filename) {
	std::ofstream outFile(filename); 
	if (!outFile.is_open()) {
		std::cerr << "�޷����ļ�����д��" << std::endl;
		return;
	}

	outFile << number << endl;
	// д�����ݵ��ļ�
	for (unsigned int i = 0; i < number; i++) {
		outFile << data[i].ReadData_string_number(36) << std::endl;
	}

	// �ر��ļ�
	outFile.close();
}

// ���ļ��ж�ȡ
unsigned int read_from_file(MyData*& data, string filename) {
	std::ifstream inFile(filename); // ���ļ����ڶ�ȡ
	if (!inFile.is_open()) {
		std::cerr << "�޷����ļ����ж�ȡ" << std::endl;
		return 0;
	}

	unsigned int number;
	if (!(inFile >> number)) {
		std::cerr << "��ȡ����ʧ��" << std::endl;
		return 0;
	}

	try {
		data = new MyData[number];
		for (unsigned int i = 0; i < number; i++) {
			std::string line;
			if (!(inFile >> line)) {
				throw std::runtime_error("��ȡ����ʧ��");
			}
			data[i].WriteData_string_number(line, " input ", 36);
			//cout << "line:" << line << endl;
		}
	}
	catch (const std::exception& e) {
		std::cerr << "�쳣: " << e.what() << std::endl;
		delete[] data; // �ͷ��ѷ�����ڴ�
		data = nullptr; // ����Ұָ��
		return 0;
	}

	inFile.close(); // �ر��ļ�
	return number;
}