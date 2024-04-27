#pragma GCC optimize(3)

#pragma once
//<>
#include <iostream>
#include <gmp.h>
#include <fstream>
#include <filesystem>
#include <vector>
#include <string>

//""
#include "file.h"
#include "struct.h"
#include "encrypt.h"
#include "compress.h"

namespace fs = std::filesystem;
using namespace std;

void opening_menu();

int main() {
	/*
	MyData test, en, de, n, e, d;
	test.WriteData_string("nihao!你好！");
	cout << "test_:" << test.ReadData_string() << endl;
	cout << "test_number:" << test.ReadData_string_number() << endl;

	keygen(n.data, e.data, d.data);
	cout << "n:" << n.ReadData_string_number() << endl;
	cout << "e:" << e.ReadData_string_number() << endl;
	cout << "d:" << d.ReadData_string_number() << endl;

	string filename = "test";
	SaveKeys(filename);
	readMpzsFromFile(n.data, e.data, filename + ".PublicKey");
	readMpzsFromFile(n.data, d.data, filename + ".PrivateKey");

	encrypt(test.data, test.data, n.data, e.data);
	cout << "en_number:" << test.ReadData_string_number() << endl;
	mpz_to_binary(test.data, "a.t");
	binary_to_mpz("a.t", test.data);

	decrypt(test.data, test.data, n.data, d.data);
	cout << "de_:" << test.ReadData_string() << endl;
	cout << "de_number:" << test.ReadData_string_number() << endl;
	*/

	/*
	unsigned int n = 2;
	MyData* myDataArray = new MyData[n];
	myDataArray[0].WriteData_string("zzzzzzz12345");
	myDataArray[1].WriteData_string("qwert1111111111111");

	cout << myDataArray[1].ReadData_string_number(36) << endl;
	// 定义文件名
	std::string filename = "data.bin";
	write_to_file(myDataArray, n, filename);

	MyData* myData_;
	read_from_file(myData_, filename);
	for (unsigned int i = 0; i < n; i++) {
		cout << myData_[i].ReadData_string(36) << endl;
	}

	delete[] myData_, myDataArray;
	*/
	int chouse;
	while (true) {
		system("cls");
		chouse = 0;
		opening_menu();
		cin >> chouse;
		switch (chouse) {

			//生成并保存一对私钥、密钥
		case 1: {
			string* KeyName = new string;
			cout << "请输入密钥名(将用于保存与识别)：";
			cin >> *KeyName;
			cout << "正在生成密钥\n这可能会用十几秒钟的时间" << endl;
			SaveKeys(*KeyName);

			delete KeyName;
			system("pause");
			break;
		}

			//加密一组文本
		case 2: {
			cout << "请输入将要加密的文本数量：";
			unsigned int text_quantity = 0;
			cin >> text_quantity;
			int* password_chouse = new int[text_quantity];
			MyData* texts = new MyData[text_quantity];
			MyData* check_this = new MyData;

			unsigned int key_number = 0;
			fs::path currentDir = fs::current_path();
			// 遍历当前目录
			for (const auto& entry : fs::directory_iterator(currentDir)) {
				// 检查文件扩展名是否为 .PublicKey
				if (entry.path().extension() == ".PublicKey") {
					++key_number;
				}
			}
			std::cout << "找到了 " << key_number << " 个 .PublicKey 文件。" << std::endl;
			MyData* n = new MyData[key_number];
			MyData* e = new MyData[key_number];
			string* file_name = new string[key_number];
			// 遍历当前目录下的所有.PublicKey文件
			int _PublicKey_i = 0;
			for (const auto& entry : filesystem::directory_iterator(".")) {
				if (entry.path().extension() == ".PublicKey") {
					file_name[_PublicKey_i] = entry.path().filename().string();
					// 读取公钥
					readMpzsFromFile(n[_PublicKey_i].data , e[_PublicKey_i].data, entry.path().filename().string());
					_PublicKey_i++;
				}
			}

			// 获取密钥与文本
			for (int i = 0; i < text_quantity; i++) {
				system("cls");
				cout << "第" << i + 1 << "个：" << endl;
				// 输出所有读取到的公钥信息
				for (int j = 0; j < key_number; j++) {
					cout << "[" << j + 1 << "]";
					cout << "文件名: " << file_name[j] << "\n公钥";
					//cout << ":(" << n[j].ReadData_string_number() << ";" << e[j].ReadData_string_number();
					cout << endl;
				}
				cout << "请输入想选择的密钥：" << endl;
				cin >> password_chouse[i];
				password_chouse[i] = (password_chouse[i] - 1) % key_number;//确保不会溢出
				cout << "请输入想加密的文本(按回车结束)：";
				cin.ignore(numeric_limits<streamsize>::max(), '\n'); // 清除输入流中的换行符
				string input_text;
				getline(cin,input_text);
				texts[i].WriteData_string(input_text);
			}

			MyData* all = new MyData[text_quantity * 2];
			check_this->WriteData_string("this");
			for (int i = 0; i < text_quantity; i++) {
				encrypt(all[i * 2].data, check_this->data, n[password_chouse[i]].data, e[password_chouse[i]].data);
				encrypt(all[(i * 2) + 1].data, texts[i].data, n[password_chouse[i]].data, e[password_chouse[i]].data);
			}

			string save_filename;
			cout << "\n想要保存到的文件名：";
			cin >> save_filename;
			write_to_file(all, (text_quantity * 2), (save_filename + ".TEPF"));
			cout << "保存成功" << endl;
			system("pause");

			delete[] n, e, all, file_name, texts, password_chouse;
			delete check_this;

			break;
		}

			//解密一组文本
		case 3: {

			// 选择解密文件
			unsigned int file_number = 0;
			fs::path currentDir_file = fs::current_path();
			// 遍历当前目录
			for (const auto& entry : fs::directory_iterator(currentDir_file)) {
				// 检查文件扩展名是否为 .PublicKey
				if (entry.path().extension() == ".TEPF") {
					++file_number;
				}
			}
			std::cout << "找到了 " << file_number << " 个 加密 文件" << std::endl;

			string* file_name = new string[file_number];
			int _file_name_i = 0;
			cout << "文件名分别为:\n";
			for (const auto& entry : filesystem::directory_iterator(".")) {
				if (entry.path().extension() == ".TEPF") {
					file_name[_file_name_i] = entry.path().filename().string();
					cout << "第[" << (_file_name_i + 1) << "]个:" << entry.path().filename().string() << endl;
					_file_name_i++;
				}
			}
			unsigned int file_chouse = 0;
			cout << "请选择文件(输入序号):";
			cin >> file_chouse; 
			file_chouse = (file_chouse - 1) % file_number;

			//---------------------------------------------------------------------
			
			// 选择解密密钥
			unsigned int key_number = 0;
			fs::path currentDir_key = fs::current_path();
			// 遍历当前目录
			for (const auto& entry : fs::directory_iterator(currentDir_key)) {
				// 检查文件扩展名是否为 .PrivateKey
				if (entry.path().extension() == ".PrivateKey") {
					++key_number;
				}
			}
			std::cout << "找到了 " << key_number << " 个 私钥 文件" << std::endl;

			string* key_name = new string[key_number];
			int _key_name_i = 0;
			cout << "私钥名分别为:\n";
			for (const auto& entry : filesystem::directory_iterator(".")) {
				if (entry.path().extension() == ".PrivateKey") {
					key_name[_key_name_i] = entry.path().filename().string();
					cout << "第[" << (_key_name_i + 1) << "]个:" << entry.path().filename().string() << endl;
					_key_name_i++;
				}
			}
			unsigned int key_chouse = 0;
			cout << "请选择文件将用私钥(输入序号):";
			cin >> key_chouse; 
			key_chouse = (key_chouse - 1) % key_number;
			// 读取私钥
			MyData n, d;
			readMpzsFromFile(n.data, d.data, key_name[key_chouse]);

			// 寻找标识体并解密
			unsigned int data_mumber = 0;
			MyData* allData_;
			data_mumber = read_from_file(allData_, file_name[file_chouse]);
			MyData check_this, result;
			for (unsigned int i = 0; i < data_mumber; i++) {
				decrypt(check_this.data, allData_[i].data, n.data, d.data);
				if (check_this.ReadData_string() == "this") {
					i++;
					decrypt(result.data, allData_[i].data, n.data, d.data);
					break;
				}
			}
			cout << "-------------------\n结果:" << result.ReadData_string() << endl << endl;

			system("pause");

			delete[] file_name, key_name, allData_;

			break;
		}

		default:{
			std::cout << "没有这个选项呦~\n";
			system("pause");
			break;
		}

		}
	}
	return 0;
}

void opening_menu() {
	cout << "1:生成并保存一对私钥、密钥\n";
	cout << "2:加密一组文本\n";
	cout << "3:解密一组文本\n";
	cout << "4:暂无\n";
}