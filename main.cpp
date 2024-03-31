#include <iostream>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <time.h>
#include <gmp.h>
#include <gmpxx.h>
#include <string>
#include <vector>
#include <codecvt>
#include <iconv.h>
#include <fstream>
#include <sstream>
#include <cstring>
#include <zlib.h>
#include <filesystem>

using namespace std;

//��Կ�ļ���ȡ
struct PublicKeyInfo {
    std::string fileName;
    mpz_t n;
    mpz_t e;

    // ���캯��
    PublicKeyInfo(const std::string& fName) : fileName(fName) {
        mpz_inits(n, e, NULL);
    }

    // �������캯��
    PublicKeyInfo(const PublicKeyInfo& other) : fileName(other.fileName) {
        mpz_inits(n, e, NULL);
        mpz_set(n, other.n);
        mpz_set(e, other.e);
    }

    // ��ֵ�����
    PublicKeyInfo& operator=(const PublicKeyInfo& other) {
        if (this != &other) {
            fileName = other.fileName;
            mpz_set(n, other.n);
            mpz_set(e, other.e);
        }
        return *this;
    }

    // ��������
    ~PublicKeyInfo() {
        mpz_clear(n);
        mpz_clear(e);
    }
};

//˽Կ�ļ���ȡ
struct PrivateKeyInfo {
    std::string fileName;
    mpz_t n;
    mpz_t d;

    // ���캯��
    PrivateKeyInfo(const std::string& fName) : fileName(fName) {
        mpz_inits(n, d, NULL);
    }

    // �������캯��
    PrivateKeyInfo(const PrivateKeyInfo& other) : fileName(other.fileName) {
        mpz_inits(n, d, NULL);
        mpz_set(n, other.n);
        mpz_set(d, other.d);
    }

    // ��ֵ�����
    PrivateKeyInfo& operator=(const PrivateKeyInfo& other) {
        if (this != &other) {
            fileName = other.fileName;
            mpz_set(n, other.n);
            mpz_set(d, other.d);
        }
        return *this;
    }

    // ��������
    ~PrivateKeyInfo() {
        mpz_clear(n);
        mpz_clear(d);
    }
};

int DebugText = 1;// 0�� 1��

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

    if (DebugText == 1) cout << "�����ѳɹ�д���ļ�" << endl;
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
    mpz_ui_pow_ui(min, 2, 601); // ����minΪ2^601
    mpz_ui_pow_ui(max, 2, 602); // ����maxΪ2^602
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
void encrypt(mpz_t c, mpz_t m, mpz_t n, mpz_t e) {
    // ����c = m^e mod n
    mod_pow(c, m, e, n); // ʹ��gmp�ṩ��ģ�ݺ���
}
void encrypt(mpz_class& result, const mpz_class& message, const mpz_class& n, const mpz_class& e) {
    mpz_powm(result.get_mpz_t(), message.get_mpz_t(), e.get_mpz_t(), n.get_mpz_t());
}

// һ����������ʹ��RSA����һ����Ϣ
void decrypt(mpz_t m, mpz_t c, mpz_t n, mpz_t d) {
    // ����m = c^d mod n
    mod_pow(m, c, d, n); // ʹ��gmp�ṩ��ģ�ݺ���
}
void decrypt(mpz_class& result, const mpz_class& message, const mpz_class& n, const mpz_class& d) {
    // ����m = c^d mod n
    mpz_powm(result.get_mpz_t(), message.get_mpz_t(), d.get_mpz_t(), n.get_mpz_t());
}

// ѹ���ַ���
mpz_class compressString(const std::string& str) {
    // ʹ�� GMP ���е� mpz_class �����洢����������
    mpz_class original(0); // ��ʼ��Ϊ0

    // ʹ�� zlib ����ѹ��
    z_stream zs;
    memset(&zs, 0, sizeof(zs));

    if (deflateInit(&zs, Z_DEFAULT_COMPRESSION) != Z_OK) {
        return original; // ��ʼ��ʧ��
    }

    zs.next_in = (Bytef*)str.data();
    zs.avail_in = str.size();

    std::vector<char> buffer(1024);

    do {
        zs.next_out = (Bytef*)buffer.data();
        zs.avail_out = buffer.size();

        int status = deflate(&zs, Z_FINISH);
        if (status == Z_OK || status == Z_STREAM_END) {
            for (size_t i = 0; i < buffer.size() - zs.avail_out; ++i) {
                original <<= 8; // ����8λ���ڳ��ռ�洢��һ���ֽ�
                original += (unsigned char)buffer[i]; // ����һ���ֽ���ӵ���������
            }
        }
        else {
            deflateEnd(&zs);
            return original; // ѹ��ʧ��
        }
    } while (zs.avail_out == 0);

    deflateEnd(&zs);
    return original;
}

//��ѹ�ַ���
std::string decompressString(const mpz_class& compressedInt) {
    mpz_class original(compressedInt);
    std::vector<unsigned char> byteVector;
    while (original > 0) {
        unsigned char byte = original.get_ui();
        byteVector.insert(byteVector.begin(), byte);
        original >>= 8;
    }
    std::string decompressedString(byteVector.begin(), byteVector.end());

    std::string result;

    z_stream zs;
    memset(&zs, 0, sizeof(zs));
    if (inflateInit(&zs) != Z_OK) {
        return "��ʼ��ʧ��";
    }

    int status = Z_OK;
    size_t totalBytesRead = 0;
    size_t bufferSize = 1024;  // ��ʼ��������С

    do {
        std::vector<char> buffer(bufferSize);
        zs.next_in = (Bytef*)(decompressedString.data() + totalBytesRead);
        zs.avail_in = decompressedString.size() - totalBytesRead;

        zs.next_out = (Bytef*)buffer.data();
        zs.avail_out = buffer.size();

        status = inflate(&zs, Z_FINISH);

        if (status == Z_OK || status == Z_STREAM_END) {
            result.append(buffer.data(), buffer.size() - zs.avail_out);
        }
        else if (status == Z_BUF_ERROR) {
            // ���������������·������Ļ����������³�ʼ�� zlib ������
            bufferSize *= 2;
            inflateEnd(&zs);
            memset(&zs, 0, sizeof(zs));
            if (inflateInit(&zs) != Z_OK) {
                return "��ʼ��ʧ��";
            }
            totalBytesRead = 0; // �����Ѷ�ȡ���ֽ���
            continue;
        }
        else {
            inflateEnd(&zs);
            return "��ѹʧ��";
        }

        totalBytesRead += decompressedString.size() - totalBytesRead - zs.avail_in;
    } while (status != Z_STREAM_END);

    inflateEnd(&zs);
    return result;
}

// д�� mpz_class ���󵽶������ļ�
void writeBigIntToFile(const mpz_class& number, const std::string& filename) {
    // ��ȡ mpz_class ����Ĵ�С�����ֽ�Ϊ��λ��
    size_t dataSize = mpz_sizeinbase(number.get_mpz_t(), 2) / 8 + 1;

    // �����㹻�Ŀռ����洢����������
    unsigned char* dataBuffer = new unsigned char[dataSize];

    // �� mpz_class ����ת��Ϊ����������
    mpz_export(dataBuffer, nullptr, 1, 1, 1, 0, number.get_mpz_t());

    // ���ļ���д�����������
    std::ofstream outFile(filename, std::ios::binary);
    if (!outFile) {
        std::cerr << "�޷����ļ�" << std::endl;
        delete[] dataBuffer;
        return;
    }

    // д����������ݵ��ļ�
    outFile.write(reinterpret_cast<char*>(dataBuffer), dataSize);

    // �ر��ļ�
    outFile.close();

    // �ͷ��ڴ�
    delete[] dataBuffer;

    std::cout << "�����ѳɹ�д���ļ�" << std::endl;
}

// �Ӷ������ļ��ж�ȡ���ݲ����ض�Ӧ�� mpz_class ����
mpz_class readBigIntFromFile(const std::string& filename) {
    // ���ļ��Զ�ȡ����������
    std::ifstream inFile(filename, std::ios::binary);
    if (!inFile) {
        std::cerr << "�޷����ļ�" << std::endl;
        return 0; // �������ʾ��ȡʧ��
    }

    // ��ȡ�ļ���С
    inFile.seekg(0, std::ios::end);
    std::streampos fileSize = inFile.tellg();
    inFile.seekg(0, std::ios::beg);

    // �����㹻�Ŀռ����洢����������
    unsigned char* dataBuffer = new unsigned char[fileSize];

    // ���ļ��ж�ȡ����������
    inFile.read(reinterpret_cast<char*>(dataBuffer), fileSize);

    // �ر��ļ�
    inFile.close();

    // ����һ�� mpz_class ���󲢸�ֵ
    mpz_class number;

    // ������������ת��Ϊ mpz_class ����
    mpz_import(number.get_mpz_t(), fileSize, 1, 1, 1, 0, dataBuffer);

    // �ͷ��ڴ�
    delete[] dataBuffer;

    return number;
}

// ����RSA����Կ��
void generateRSAKeys(mpz_t& n, mpz_t& e, mpz_t& d) {
    mpz_inits(n, e, d, NULL);
    keygen(n, e, d);
}

// ���ַ���ת��Ϊ��������
vector<uint32_t> convertStringToNumbers(const string& str) {
    vector<uint32_t> plaintext;
    for (size_t i = 0; i < str.size(); ++i) {
        unsigned char c = str[i];
        plaintext.push_back((uint32_t)c);
    }
    return plaintext;
}

// �����������ӡΪʮ������
void printNumbersAsHex(const vector<uint32_t>& numbers) {
    cout << "UTF-8 ����ת��Ϊ���֣�";
    for (uint32_t num : numbers) {
        cout << hex << num << " ";
    }
    cout << endl;
}

// �����������鲢��ӡ���ܽ��
void encryptAndPrint(vector<uint32_t>& plaintext, mpz_t& n, mpz_t& e) {
    const int block_size = mpz_sizeinbase(n, 2) / 8 - 1;
    cout << "ÿ��С��ĳ���: " << block_size << " �ֽ�" << endl;

    mpz_t ciphertext;
    mpz_init(ciphertext);

    // ���� n �Ĵ�С��ʼ�� _plaintext
    size_t expected_ciphertext_size = mpz_sizeinbase(n, 2) / 32 + 1; // ÿ�� uint32_t Ϊ 32 λ
    vector<uint32_t> _plaintext(expected_ciphertext_size * block_size, 0);

    cout << "���ܽ����";
    size_t plaintext_index = 0;
    for (size_t i = 0; i < plaintext.size(); i += block_size) {
        mpz_class block(0);
        size_t len = min(block_size, (int)(plaintext.size() - i));
        for (size_t j = 0; j < len; ++j) {
            block <<= 8;
            block += plaintext[i + j];
        }
        encrypt(ciphertext, block.get_mpz_t(), n, e);
        gmp_printf("%ZX", ciphertext);

        // �� ciphertext ��ֵת��Ϊ uint32_t ���洢�� _plaintext
        size_t count;
        mpz_export(&_plaintext[plaintext_index], &count, -1, sizeof(uint32_t), 0, 0, ciphertext);
        plaintext_index += count;
    }
    cout << endl;

    // ����ԭʼ plaintext
    plaintext.clear();
    plaintext.insert(plaintext.end(), _plaintext.begin(), _plaintext.begin() + plaintext_index);

    mpz_clear(ciphertext);
}

// ѹ���ַ�����д���ļ�
void compressAndWriteToFile(const string& str) {
    mpz_class compressed = compressString(str);
    writeBigIntToFile(compressed, "Test.TEPF");
}

// ���ļ��ж�ȡ����ѹ���ַ���
string readAndDecompressFromFile() {
    mpz_class read_compressed = readBigIntFromFile("Test.TEPF");
    return decompressString(read_compressed);
}

// ��ʼ�˵�
void opening_menu() {
    cout << "1:���ɲ�����һ��˽Կ����Կ\n";
    cout << "2:����һ���ı�\n";
    cout << "3:����һ���ı�\n";
    cout << "4:����\n";
}

// ������Կ���ֱ𱣴�
void SaveKeys(string key_name) {
    // ����
    mpz_t n, e, d;
    generateRSAKeys(n, e, d);

    // ����
    writeMpzsToFile(n, e, key_name + ".PublicKey");
    writeMpzsToFile(n, d, key_name + ".PrivateKey");

    if (DebugText == 1) {
        // ��ӡ��Կ��˽Կ
        gmp_printf("��Կ: (%Zd, %Zd)\n", n, e);
        gmp_printf("˽Կ: (%Zd, %Zd)\n", n, d);
    }

    mpz_clears(n, e, d, NULL);
    cout << "����ɹ�\n";
}

// ����������д���ļ�
void writeData(const vector<vector<uint32_t>>& data, const string& filename, const vector<mpz_class>& public_n, const vector<mpz_class>& public_e) {
    ofstream outFile(filename, ios::binary);
    if (!outFile) {
        cerr << "�޷����ļ�����д��" << endl;
        return;
    }

    if (data.size() != public_n.size() || data.size() != public_e.size()) {
        cerr << "��Կ���������ݿ�������ƥ��" << endl;
        return;
    }

    for (size_t i = 0; i < data.size(); ++i) {
        const auto& entry = data[i];
        const mpz_class& n = public_n[i];
        const mpz_class& e = public_e[i];

        // ��uint32_t����ת��Ϊ�ַ���
        string str(entry.begin(), entry.end());
        if (DebugText == 1)cout << "str:" << str << endl;

        // ѹ���ַ���
        mpz_class compressed = compressString(str);

        // ��ȡѹ���ַ����Ķ����Ʊ�ʾ
        size_t size = (mpz_sizeinbase(compressed.get_mpz_t(), 2) + CHAR_BIT - 1) / CHAR_BIT;
        char* buffer = new char[size];
        mpz_export(buffer, nullptr, 1, 1, 0, 0, compressed.get_mpz_t());

        // �� "this" ת��Ϊ mpz_class ������
        mpz_class this_str("74686973", 16); // "this" ��ʮ�����Ʊ�ʾ
        mpz_class encrypted_this;
        encrypt(encrypted_this, this_str, n, e);

        // ��ȡ���ܺ� "this" �Ķ����Ʊ�ʾ
        size_t encrypted_this_size = (mpz_sizeinbase(encrypted_this.get_mpz_t(), 2) + CHAR_BIT - 1) / CHAR_BIT;
        char* encrypted_this_buffer = new char[encrypted_this_size];
        mpz_export(encrypted_this_buffer, nullptr, 1, 1, 0, 0, encrypted_this.get_mpz_t());

        // д����ܺ�� "this"����
        outFile.write(reinterpret_cast<const char*>(&encrypted_this_size), sizeof(size_t));

        // д����ܺ�� "this"
        outFile.write(encrypted_this_buffer, encrypted_this_size);

        // д��ѹ�����ݳ���
        outFile.write(reinterpret_cast<const char*>(&size), sizeof(size_t));

        // д��ѹ�����ݱ���
        outFile.write(buffer, size);

        if (DebugText == 1) {
            gmp_printf("compressed��%ZX\n", compressed.get_mpz_t()); // ���ʮ���Ʊ�ʾ�� mpz_class ����
            std::cout << "buffer (hex): ";
            for (size_t i = 0; i < size; ++i) {
                std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(static_cast<unsigned char>(buffer[i])); // ��ʮ��������ʽ���ÿ���ֽ�
            }
        }

        // �ͷ��ڴ�
        delete[] buffer;
        delete[] encrypted_this_buffer;
    }

    outFile.close();
}

// ���������ݴ��ļ���ȡ
vector<string> readAndDecompressData(const string& filename, const vector<mpz_class>& private_n, const vector<mpz_class>& private_d) {
    vector<string> decompressed_data;

    ifstream inFile(filename, ios::binary);
    if (!inFile) {
        cerr << "�޷����ļ����ж�ȡ" << endl;
        return decompressed_data;
    }

    size_t _size;
    size_t size;
    while (inFile.read(reinterpret_cast<char*>(&_size), sizeof(size_t))) {
        char* cheac_this = new char[_size];
        inFile.read(cheac_this, _size);

        // ��ȡ���ܺ�� "this"
        mpz_class encrypted_this;
        mpz_import(encrypted_this.get_mpz_t(), _size, 1, sizeof(char), 0, 0, cheac_this);
        if (DebugText == 1)gmp_printf("encrypted_this:%Zd\n", encrypted_this);
        //inFile.read(reinterpret_cast<char*>(encrypted_this.get_mpz_t()), encrypted_this.get_mpz_t()->_mp_alloc * sizeof(mp_limb_t));

        // ���Խ��� "this"
        mpz_class decrypted_this;
        bool decryption_successful = false;
        for (size_t i = 0; i < private_n.size(); ++i) {
            decrypt(decrypted_this, encrypted_this, private_n[i], private_d[i]);
            if (DebugText == 1)gmp_printf("decrypted_this:%Zd\n", decrypted_this);
            if (decrypted_this == mpz_class("74686973", 16)) { // "this" ��ʮ�����Ʊ�ʾ
                decryption_successful = true;
                break;
            }
        }

        // ������ܳɹ����������ѹ������
        if (decryption_successful) {
            // ��ȡѹ�����ݳ���
            size_t size;
            inFile.read(reinterpret_cast<char*>(&size), sizeof(size_t));

            // �����ڴ��Դ洢ѹ������
            char* buffer = new char[size];

            // ��ȡѹ�����ݱ���
            inFile.read(buffer, size);

            // ����ȡ�Ķ���������ת��Ϊ mpz_class ����
            mpz_class compressed;
            mpz_import(compressed.get_mpz_t(), size, 1, sizeof(char), 0, 0, buffer);
            if (DebugText == 1)gmp_printf("compressed��%ZX\n", compressed.get_mpz_t());

            // �ͷ��ڴ�
            delete[] buffer;

            // ��ѹ������
            string decompressed_data_piece = decompressString(compressed);
            cout << "decompressed_data_piece: " << decompressed_data_piece << endl;

            // ����decompressed_data�ռ��С
            decompressed_data.reserve(decompressed_data.size() + decompressed_data_piece.size() + 1);
            decompressed_data.push_back(decompressed_data_piece);

        }
        delete[] cheac_this;
    }
    inFile.close();
    return decompressed_data;
}

int main() {
    int chouse;
    while (true){
        //system("cls");
        // ��ʼ�˵�
        chouse = 0;
        opening_menu();
        cin >> chouse;
        switch (chouse){
        
        //���ɲ�����һ��˽Կ����Կ
        case 1: {
            string* KeyName = new string;
            cout << "��������Կ��(�����ڱ�����ʶ��)��";
            cin >> *KeyName;
            SaveKeys(*KeyName);

            //Debug
            if (DebugText == 1) {
                mpz_t n, e, d;
                mpz_inits(n, e, d, NULL);
                readMpzsFromFile(n, e, *KeyName + ".PublicKey");
                readMpzsFromFile(n, d, *KeyName + ".PrivateKey");
                gmp_printf("��Կ: (%Zd, %Zd)\n", n, e);
                gmp_printf("˽Կ: (%Zd, %Zd)\n", n, d);
                mpz_clears(n, e, d, NULL);
            }

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
            string* texts = new string[text_quantity];

            // ��ȡ��Կ
            std::vector<PublicKeyInfo> publicKeys;

            // ������ǰĿ¼�µ�����.PublicKey�ļ�
            for (const auto& entry : filesystem::directory_iterator(".")) {
                if (entry.path().extension() == ".PublicKey") {
                    PublicKeyInfo keyInfo(entry.path().filename().string());

                    // ��ȡ��Կ
                    readMpzsFromFile(keyInfo.n, keyInfo.e, keyInfo.fileName);

                    // ���ṹ����ӵ�������
                    publicKeys.push_back(keyInfo);
                }
            }

            // ��ȡ��Կ���ı�
            for (int i = 0;i < text_quantity;i++){
                system("cls");
                cout << "��" << i + 1 << "����" << endl;
                cout << "��������ѡ�����Կ��" << endl;
                // ������ж�ȡ���Ĺ�Կ��Ϣ
                int n_keyInfo = 1;
                for (const auto& keyInfo : publicKeys) {
                    gmp_printf("�ļ���: %s\n��Կ: (%Zd, %Zd)\n", keyInfo.fileName.c_str(), keyInfo.n, keyInfo.e);
                    //gmp_printf("��%d��-> �ļ���: %s\n", n_keyInfo++, keyInfo.fileName.c_str());
                }
                cin >> password_chouse[i];
                cout << "����������ܵ��ı�(���س�����)��";
                cin.ignore(numeric_limits<streamsize>::max(), '\n'); // ����������еĻ��з�
                getline(cin, texts[i]);
            }

            // ���м���
            vector<vector<uint32_t>> allPlaintexts;
            for (int i = 0; i < text_quantity; i++) {
                mpz_t n, e;
                mpz_init(n);
                mpz_init(e);
                int check_key = 0;
                vector<uint32_t> plaintext = convertStringToNumbers(texts[i]);
                if (DebugText == 1) {
                    cout << "�������ݣ�";
                    for (const auto& num : plaintext) {
                        cout << num << " ";
                    }
                    cout << endl;
                }
                mpz_set(n, publicKeys[password_chouse[i] - 1].n);
                mpz_set(e, publicKeys[password_chouse[i] - 1].e);
                encryptAndPrint(plaintext, n, e);
                allPlaintexts.push_back(plaintext);

                mpz_clears(n, e, NULL);
            }

            // ���Ϲ�Կ
            // ����ȡ�Ĺ�Կ���ϵ� vector<mpz_class> ��
            vector<mpz_class> public_n;
            vector<mpz_class> public_e;
            for (int i = 0; i < text_quantity; i++) {
                if ((password_chouse[i] - 1) >= 0 && (password_chouse[i] - 1) < publicKeys.size()) { // ��������Ƿ���Ч
                    // �� mpz_t ת��Ϊ mpz_class ����
                    mpz_class mpz_n(publicKeys[(password_chouse[i] - 1)].n);
                    mpz_class mpz_e(publicKeys[(password_chouse[i] - 1)].e);

                    public_n.push_back(mpz_n);
                    public_e.push_back(mpz_e);
                }
                else {

                    cerr << "��Կѡ����Ч��" << (password_chouse[i] - 1) << endl;
                }
            }

            // д�����ݵ��ļ�
            string save_filename;
            cout << "\n��Ҫ���浽���ļ�����";
            cin >> save_filename;
            writeData(allPlaintexts, save_filename + ".TEPF", public_n, public_e);
            cout << "����ɹ�" << endl;
            system("pause");

            delete[] password_chouse, texts;

            break;
        }

        //Ѱ�Ҳ�����һ���ı�
        case 3: {

            // ��ȡ��Կ
            std::vector<PrivateKeyInfo> privateKeys;

            // ������ǰĿ¼�µ�����.PrivateKey�ļ�
            for (const auto& entry : filesystem::directory_iterator(".")) {
                if (entry.path().extension() == ".PrivateKey") {
                    PrivateKeyInfo keyInfo(entry.path().filename().string());

                    // ��ȡ��Կ
                    readMpzsFromFile(keyInfo.n, keyInfo.d, keyInfo.fileName);

                    // ���ṹ����ӵ�������
                    privateKeys.push_back(keyInfo);
                }
            }

            // ��ȡ��Կѡ��
            int password_chouse = 1;
            //system("cls");
            cout << "��������ѡ�����Կ��" << endl;
            // ������ж�ȡ���Ĺ�Կ��Ϣ
            int n_keyInfo = 1;
            for (const auto& keyInfo : privateKeys) {
                gmp_printf("�ļ���: %s\n��Կ: (%Zd, %Zd)\n", keyInfo.fileName.c_str(), keyInfo.n, keyInfo.d);
                //gmp_printf("��%d��-> �ļ���: %s\n", n_keyInfo++, keyInfo.fileName.c_str());
            }
            cin >> password_chouse;

            mpz_t n, d;
            mpz_init(n);
            mpz_init(d);
            mpz_set(n, privateKeys[password_chouse - 1].n);
            mpz_set(d, privateKeys[password_chouse - 1].d);
            mpz_class mpz_n(privateKeys[(password_chouse - 1)].n);
            mpz_class mpz_d(privateKeys[(password_chouse - 1)].d);
            mpz_clears(n, d, NULL);

            // ������������Ԫ�ص� vector
            std::vector<mpz_class> private_n_vector{ mpz_n };
            std::vector<mpz_class> private_d_vector{ mpz_d };

            // ���ļ���ȡ����ѹ������
            vector<string> decompressedData = readAndDecompressData("1.TEPF", private_n_vector, private_d_vector);
            if (DebugText == 1) {
                cout << "decompressedData��";
                for (const auto& num : decompressedData) {
                    cout << num << " ";
                }
                cout << endl;
            }

            // ���ܲ���
            for (auto& str : decompressedData) {
                // �� string ת��Ϊ char* ����
                const char* encryptedData = str.c_str();
                size_t encryptedSize = str.size();

                // ����һ�� mpz_class ���󲢸�ֵ
                mpz_class number;

                // ������������ת��Ϊ mpz_class ����
                mpz_import(number.get_mpz_t(), encryptedSize, 1, 1, 1, 0, encryptedData);

                // ʹ��˽Կ���н���
                mpz_class decryptedData;
                decrypt(decryptedData, number, mpz_n, mpz_d);

                // �����ܺ�����ݸ��µ�ԭʼ�ַ�����
                str = mpz_get_str(nullptr, 16, decryptedData.get_mpz_t()); // �� mpz_class ת��Ϊ string
            }

            // �����ѹ���������
            for (const auto& str : decompressedData) {
                cout << "�����" << str << endl;
            }

            system("pause");
            break;
        }
        default:
            cout << "û�����ѡ����~\n";
            break;
        }
    }
    /*
    string words;
    cout << "������Ҫ���ܵ��ַ�����";
    getline(cin, words);

    mpz_t n, e, d;
    generateRSAKeys(n, e, d);

    // ��ӡ��Կ��˽Կ
    gmp_printf("��Կ: (%Zd, %Zd)\n", n, e);
    gmp_printf("˽Կ: (%Zd, %Zd)\n", n, d);

    // ���ַ���ת��Ϊ��������
    vector<uint32_t> plaintext = convertStringToNumbers(words);

    // ��ӡ���������ʮ��������ʽ
    printNumbersAsHex(plaintext);

    // �����������鲢��ӡ���ܽ��
    encryptAndPrint(plaintext, n, e);

    // ��ѹ������ַ���д���ļ�
    compressAndWriteToFile(words);

    // ���ļ��ж�ȡѹ������ַ�������ѹ��
    string decompressed = readAndDecompressFromFile();
    cout << "��ѹ����ַ�����" << decompressed << endl;

    mpz_clears(n, e, d, NULL);
    return 0;
    */
}
