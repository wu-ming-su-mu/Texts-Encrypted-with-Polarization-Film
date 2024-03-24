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

// һ����������ʹ��RSA����һ����Ϣ
void decrypt(mpz_t m, mpz_t c, mpz_t n, mpz_t d) {
    // ����m = c^d mod n
    mod_pow(m, c, d, n); // ʹ��gmp�ṩ��ģ�ݺ���
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

int main() {
    string words;
    cout << "������Ҫ���ܵ��ַ�����";
    getline(cin, words);

    vector<uint32_t> plaintext;
    for (size_t i = 0; i < words.size(); ++i) {
        unsigned char c = words[i];
        plaintext.push_back((uint32_t)c);
    }

    cout << "UTF-8 ����ת��Ϊ���֣�";
    for (uint32_t num : plaintext) {
        cout << hex << num << " ";
    }
    cout << endl;

    mpz_t n, e, d;
    mpz_inits(n, e, d, NULL);
    keygen(n, e, d);

    gmp_printf("��Կ: (%Zd, %Zd)\n", n, e);
    gmp_printf("˽Կ: (%Zd, %Zd)\n", n, d);

    const int block_size = mpz_sizeinbase(n, 2) / 8 - 1;
    cout << "ÿ��С��ĳ���: " << block_size << " �ֽ�" << endl;

    mpz_t ciphertext;
    mpz_init(ciphertext);

    cout << "���ܽ����";
    for (size_t i = 0; i < plaintext.size(); i += block_size) {
        mpz_class block(0);
        size_t len = min(block_size, (int)(plaintext.size() - i));
        for (size_t j = 0; j < len; ++j) {
            block <<= 8;
            block += plaintext[i + j];
        }
        encrypt(ciphertext, block.get_mpz_t(), n, e);
        gmp_printf("%ZX", ciphertext);
    }
    cout << endl;

    // ��ѹ������ַ���д���ļ�
    mpz_class compressed = compressString(words);
    writeBigIntToFile(compressed, "Test.TEPF");

    // ���ļ��ж�ȡѹ������ַ���
    mpz_class read_compressed = readBigIntFromFile("Test.TEPF");
    std::string decompressed = decompressString(read_compressed);
    cout << "��ѹ����ַ�����" << decompressed << endl;

    mpz_clears(n, e, d, ciphertext, NULL);
    return 0;
}
