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
	test.WriteData_string("nihao!��ã�");
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
	// �����ļ���
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

			//���ɲ�����һ��˽Կ����Կ
		case 1: {
			string* KeyName = new string;
			cout << "��������Կ��(�����ڱ�����ʶ��)��";
			cin >> *KeyName;
			cout << "����������Կ\n����ܻ���ʮ�����ӵ�ʱ��" << endl;
			SaveKeys(*KeyName);

			delete KeyName;
			system("pause");
			break;
		}

			//����һ���ı�
		case 2: {
			cout << "�����뽫Ҫ���ܵ��ı�������";
			unsigned int text_quantity = 0;
			cin >> text_quantity;
			int* password_chouse = new int[text_quantity];
			MyData* texts = new MyData[text_quantity];
			MyData* check_this = new MyData;

			unsigned int key_number = 0;
			fs::path currentDir = fs::current_path();
			// ������ǰĿ¼
			for (const auto& entry : fs::directory_iterator(currentDir)) {
				// ����ļ���չ���Ƿ�Ϊ .PublicKey
				if (entry.path().extension() == ".PublicKey") {
					++key_number;
				}
			}
			std::cout << "�ҵ��� " << key_number << " �� .PublicKey �ļ���" << std::endl;
			MyData* n = new MyData[key_number];
			MyData* e = new MyData[key_number];
			string* file_name = new string[key_number];
			// ������ǰĿ¼�µ�����.PublicKey�ļ�
			int _PublicKey_i = 0;
			for (const auto& entry : filesystem::directory_iterator(".")) {
				if (entry.path().extension() == ".PublicKey") {
					file_name[_PublicKey_i] = entry.path().filename().string();
					// ��ȡ��Կ
					readMpzsFromFile(n[_PublicKey_i].data , e[_PublicKey_i].data, entry.path().filename().string());
					_PublicKey_i++;
				}
			}

			// ��ȡ��Կ���ı�
			for (int i = 0; i < text_quantity; i++) {
				system("cls");
				cout << "��" << i + 1 << "����" << endl;
				// ������ж�ȡ���Ĺ�Կ��Ϣ
				for (int j = 0; j < key_number; j++) {
					cout << "[" << j + 1 << "]";
					cout << "�ļ���: " << file_name[j] << "\n��Կ";
					//cout << ":(" << n[j].ReadData_string_number() << ";" << e[j].ReadData_string_number();
					cout << endl;
				}
				cout << "��������ѡ�����Կ��" << endl;
				cin >> password_chouse[i];
				password_chouse[i] = (password_chouse[i] - 1) % key_number;//ȷ���������
				cout << "����������ܵ��ı�(���س�����)��";
				cin.ignore(numeric_limits<streamsize>::max(), '\n'); // ����������еĻ��з�
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
			cout << "\n��Ҫ���浽���ļ�����";
			cin >> save_filename;
			write_to_file(all, (text_quantity * 2), (save_filename + ".TEPF"));
			cout << "����ɹ�" << endl;
			system("pause");

			delete[] n, e, all, file_name, texts, password_chouse;
			delete check_this;

			break;
		}

			//����һ���ı�
		case 3: {

			// ѡ������ļ�
			unsigned int file_number = 0;
			fs::path currentDir_file = fs::current_path();
			// ������ǰĿ¼
			for (const auto& entry : fs::directory_iterator(currentDir_file)) {
				// ����ļ���չ���Ƿ�Ϊ .PublicKey
				if (entry.path().extension() == ".TEPF") {
					++file_number;
				}
			}
			std::cout << "�ҵ��� " << file_number << " �� ���� �ļ�" << std::endl;

			string* file_name = new string[file_number];
			int _file_name_i = 0;
			cout << "�ļ����ֱ�Ϊ:\n";
			for (const auto& entry : filesystem::directory_iterator(".")) {
				if (entry.path().extension() == ".TEPF") {
					file_name[_file_name_i] = entry.path().filename().string();
					cout << "��[" << (_file_name_i + 1) << "]��:" << entry.path().filename().string() << endl;
					_file_name_i++;
				}
			}
			unsigned int file_chouse = 0;
			cout << "��ѡ���ļ�(�������):";
			cin >> file_chouse; 
			file_chouse = (file_chouse - 1) % file_number;

			//---------------------------------------------------------------------
			
			// ѡ�������Կ
			unsigned int key_number = 0;
			fs::path currentDir_key = fs::current_path();
			// ������ǰĿ¼
			for (const auto& entry : fs::directory_iterator(currentDir_key)) {
				// ����ļ���չ���Ƿ�Ϊ .PrivateKey
				if (entry.path().extension() == ".PrivateKey") {
					++key_number;
				}
			}
			std::cout << "�ҵ��� " << key_number << " �� ˽Կ �ļ�" << std::endl;

			string* key_name = new string[key_number];
			int _key_name_i = 0;
			cout << "˽Կ���ֱ�Ϊ:\n";
			for (const auto& entry : filesystem::directory_iterator(".")) {
				if (entry.path().extension() == ".PrivateKey") {
					key_name[_key_name_i] = entry.path().filename().string();
					cout << "��[" << (_key_name_i + 1) << "]��:" << entry.path().filename().string() << endl;
					_key_name_i++;
				}
			}
			unsigned int key_chouse = 0;
			cout << "��ѡ���ļ�����˽Կ(�������):";
			cin >> key_chouse; 
			key_chouse = (key_chouse - 1) % key_number;
			// ��ȡ˽Կ
			MyData n, d;
			readMpzsFromFile(n.data, d.data, key_name[key_chouse]);

			// Ѱ�ұ�ʶ�岢����
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
			cout << "-------------------\n���:" << result.ReadData_string() << endl << endl;

			system("pause");

			delete[] file_name, key_name, allData_;

			break;
		}

		default:{
			std::cout << "û�����ѡ����~\n";
			system("pause");
			break;
		}

		}
	}
	return 0;
}

void opening_menu() {
	cout << "1:���ɲ�����һ��˽Կ����Կ\n";
	cout << "2:����һ���ı�\n";
	cout << "3:����һ���ı�\n";
	cout << "4:����\n";
}