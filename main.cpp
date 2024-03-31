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

//公钥文件读取
struct PublicKeyInfo {
    std::string fileName;
    mpz_t n;
    mpz_t e;

    // 构造函数
    PublicKeyInfo(const std::string& fName) : fileName(fName) {
        mpz_inits(n, e, NULL);
    }

    // 拷贝构造函数
    PublicKeyInfo(const PublicKeyInfo& other) : fileName(other.fileName) {
        mpz_inits(n, e, NULL);
        mpz_set(n, other.n);
        mpz_set(e, other.e);
    }

    // 赋值运算符
    PublicKeyInfo& operator=(const PublicKeyInfo& other) {
        if (this != &other) {
            fileName = other.fileName;
            mpz_set(n, other.n);
            mpz_set(e, other.e);
        }
        return *this;
    }

    // 析构函数
    ~PublicKeyInfo() {
        mpz_clear(n);
        mpz_clear(e);
    }
};

//私钥文件读取
struct PrivateKeyInfo {
    std::string fileName;
    mpz_t n;
    mpz_t d;

    // 构造函数
    PrivateKeyInfo(const std::string& fName) : fileName(fName) {
        mpz_inits(n, d, NULL);
    }

    // 拷贝构造函数
    PrivateKeyInfo(const PrivateKeyInfo& other) : fileName(other.fileName) {
        mpz_inits(n, d, NULL);
        mpz_set(n, other.n);
        mpz_set(d, other.d);
    }

    // 赋值运算符
    PrivateKeyInfo& operator=(const PrivateKeyInfo& other) {
        if (this != &other) {
            fileName = other.fileName;
            mpz_set(n, other.n);
            mpz_set(d, other.d);
        }
        return *this;
    }

    // 析构函数
    ~PrivateKeyInfo() {
        mpz_clear(n);
        mpz_clear(d);
    }
};

int DebugText = 1;// 0关 1开

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

    if (DebugText == 1) cout << "数据已成功写入文件" << endl;
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
    mpz_ui_pow_ui(min, 2, 601); // 设置min为2^601
    mpz_ui_pow_ui(max, 2, 602); // 设置max为2^602
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
void encrypt(mpz_t c, mpz_t m, mpz_t n, mpz_t e) {
    // 计算c = m^e mod n
    mod_pow(c, m, e, n); // 使用gmp提供的模幂函数
}
void encrypt(mpz_class& result, const mpz_class& message, const mpz_class& n, const mpz_class& e) {
    mpz_powm(result.get_mpz_t(), message.get_mpz_t(), e.get_mpz_t(), n.get_mpz_t());
}

// 一个函数用于使用RSA解密一条消息
void decrypt(mpz_t m, mpz_t c, mpz_t n, mpz_t d) {
    // 计算m = c^d mod n
    mod_pow(m, c, d, n); // 使用gmp提供的模幂函数
}
void decrypt(mpz_class& result, const mpz_class& message, const mpz_class& n, const mpz_class& d) {
    // 计算m = c^d mod n
    mpz_powm(result.get_mpz_t(), message.get_mpz_t(), d.get_mpz_t(), n.get_mpz_t());
}

// 压缩字符串
mpz_class compressString(const std::string& str) {
    // 使用 GMP 库中的 mpz_class 类来存储二进制数据
    mpz_class original(0); // 初始化为0

    // 使用 zlib 进行压缩
    z_stream zs;
    memset(&zs, 0, sizeof(zs));

    if (deflateInit(&zs, Z_DEFAULT_COMPRESSION) != Z_OK) {
        return original; // 初始化失败
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
                original <<= 8; // 左移8位以腾出空间存储下一个字节
                original += (unsigned char)buffer[i]; // 将下一个字节添加到大整数中
            }
        }
        else {
            deflateEnd(&zs);
            return original; // 压缩失败
        }
    } while (zs.avail_out == 0);

    deflateEnd(&zs);
    return original;
}

//解压字符串
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
        return "初始化失败";
    }

    int status = Z_OK;
    size_t totalBytesRead = 0;
    size_t bufferSize = 1024;  // 初始缓冲区大小

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
            // 缓冲区不够大，重新分配更大的缓冲区并重新初始化 zlib 流对象
            bufferSize *= 2;
            inflateEnd(&zs);
            memset(&zs, 0, sizeof(zs));
            if (inflateInit(&zs) != Z_OK) {
                return "初始化失败";
            }
            totalBytesRead = 0; // 重置已读取的字节数
            continue;
        }
        else {
            inflateEnd(&zs);
            return "解压失败";
        }

        totalBytesRead += decompressedString.size() - totalBytesRead - zs.avail_in;
    } while (status != Z_STREAM_END);

    inflateEnd(&zs);
    return result;
}

// 写入 mpz_class 对象到二进制文件
void writeBigIntToFile(const mpz_class& number, const std::string& filename) {
    // 获取 mpz_class 对象的大小（以字节为单位）
    size_t dataSize = mpz_sizeinbase(number.get_mpz_t(), 2) / 8 + 1;

    // 分配足够的空间来存储二进制数据
    unsigned char* dataBuffer = new unsigned char[dataSize];

    // 将 mpz_class 对象转换为二进制数据
    mpz_export(dataBuffer, nullptr, 1, 1, 1, 0, number.get_mpz_t());

    // 打开文件以写入二进制数据
    std::ofstream outFile(filename, std::ios::binary);
    if (!outFile) {
        std::cerr << "无法打开文件" << std::endl;
        delete[] dataBuffer;
        return;
    }

    // 写入二进制数据到文件
    outFile.write(reinterpret_cast<char*>(dataBuffer), dataSize);

    // 关闭文件
    outFile.close();

    // 释放内存
    delete[] dataBuffer;

    std::cout << "数据已成功写入文件" << std::endl;
}

// 从二进制文件中读取数据并返回对应的 mpz_class 对象
mpz_class readBigIntFromFile(const std::string& filename) {
    // 打开文件以读取二进制数据
    std::ifstream inFile(filename, std::ios::binary);
    if (!inFile) {
        std::cerr << "无法打开文件" << std::endl;
        return 0; // 返回零表示读取失败
    }

    // 获取文件大小
    inFile.seekg(0, std::ios::end);
    std::streampos fileSize = inFile.tellg();
    inFile.seekg(0, std::ios::beg);

    // 分配足够的空间来存储二进制数据
    unsigned char* dataBuffer = new unsigned char[fileSize];

    // 从文件中读取二进制数据
    inFile.read(reinterpret_cast<char*>(dataBuffer), fileSize);

    // 关闭文件
    inFile.close();

    // 创建一个 mpz_class 对象并赋值
    mpz_class number;

    // 将二进制数据转换为 mpz_class 对象
    mpz_import(number.get_mpz_t(), fileSize, 1, 1, 1, 0, dataBuffer);

    // 释放内存
    delete[] dataBuffer;

    return number;
}

// 生成RSA的密钥对
void generateRSAKeys(mpz_t& n, mpz_t& e, mpz_t& d) {
    mpz_inits(n, e, d, NULL);
    keygen(n, e, d);
}

// 将字符串转换为数字数组
vector<uint32_t> convertStringToNumbers(const string& str) {
    vector<uint32_t> plaintext;
    for (size_t i = 0; i < str.size(); ++i) {
        unsigned char c = str[i];
        plaintext.push_back((uint32_t)c);
    }
    return plaintext;
}

// 将数字数组打印为十六进制
void printNumbersAsHex(const vector<uint32_t>& numbers) {
    cout << "UTF-8 编码转换为数字：";
    for (uint32_t num : numbers) {
        cout << hex << num << " ";
    }
    cout << endl;
}

// 加密数字数组并打印加密结果
void encryptAndPrint(vector<uint32_t>& plaintext, mpz_t& n, mpz_t& e) {
    const int block_size = mpz_sizeinbase(n, 2) / 8 - 1;
    cout << "每个小块的长度: " << block_size << " 字节" << endl;

    mpz_t ciphertext;
    mpz_init(ciphertext);

    // 根据 n 的大小初始化 _plaintext
    size_t expected_ciphertext_size = mpz_sizeinbase(n, 2) / 32 + 1; // 每个 uint32_t 为 32 位
    vector<uint32_t> _plaintext(expected_ciphertext_size * block_size, 0);

    cout << "加密结果：";
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

        // 将 ciphertext 的值转换为 uint32_t 并存储到 _plaintext
        size_t count;
        mpz_export(&_plaintext[plaintext_index], &count, -1, sizeof(uint32_t), 0, 0, ciphertext);
        plaintext_index += count;
    }
    cout << endl;

    // 更新原始 plaintext
    plaintext.clear();
    plaintext.insert(plaintext.end(), _plaintext.begin(), _plaintext.begin() + plaintext_index);

    mpz_clear(ciphertext);
}

// 压缩字符串并写入文件
void compressAndWriteToFile(const string& str) {
    mpz_class compressed = compressString(str);
    writeBigIntToFile(compressed, "Test.TEPF");
}

// 从文件中读取并解压缩字符串
string readAndDecompressFromFile() {
    mpz_class read_compressed = readBigIntFromFile("Test.TEPF");
    return decompressString(read_compressed);
}

// 开始菜单
void opening_menu() {
    cout << "1:生成并保存一对私钥、密钥\n";
    cout << "2:加密一组文本\n";
    cout << "3:解密一组文本\n";
    cout << "4:暂无\n";
}

// 生成密钥并分别保存
void SaveKeys(string key_name) {
    // 生成
    mpz_t n, e, d;
    generateRSAKeys(n, e, d);

    // 保存
    writeMpzsToFile(n, e, key_name + ".PublicKey");
    writeMpzsToFile(n, d, key_name + ".PrivateKey");

    if (DebugText == 1) {
        // 打印公钥和私钥
        gmp_printf("公钥: (%Zd, %Zd)\n", n, e);
        gmp_printf("私钥: (%Zd, %Zd)\n", n, d);
    }

    mpz_clears(n, e, d, NULL);
    cout << "保存成功\n";
}

// 将加密数据写入文件
void writeData(const vector<vector<uint32_t>>& data, const string& filename, const vector<mpz_class>& public_n, const vector<mpz_class>& public_e) {
    ofstream outFile(filename, ios::binary);
    if (!outFile) {
        cerr << "无法打开文件进行写入" << endl;
        return;
    }

    if (data.size() != public_n.size() || data.size() != public_e.size()) {
        cerr << "公钥数量与数据块数量不匹配" << endl;
        return;
    }

    for (size_t i = 0; i < data.size(); ++i) {
        const auto& entry = data[i];
        const mpz_class& n = public_n[i];
        const mpz_class& e = public_e[i];

        // 将uint32_t向量转换为字符串
        string str(entry.begin(), entry.end());
        if (DebugText == 1)cout << "str:" << str << endl;

        // 压缩字符串
        mpz_class compressed = compressString(str);

        // 获取压缩字符串的二进制表示
        size_t size = (mpz_sizeinbase(compressed.get_mpz_t(), 2) + CHAR_BIT - 1) / CHAR_BIT;
        char* buffer = new char[size];
        mpz_export(buffer, nullptr, 1, 1, 0, 0, compressed.get_mpz_t());

        // 将 "this" 转换为 mpz_class 并加密
        mpz_class this_str("74686973", 16); // "this" 的十六进制表示
        mpz_class encrypted_this;
        encrypt(encrypted_this, this_str, n, e);

        // 获取加密后 "this" 的二进制表示
        size_t encrypted_this_size = (mpz_sizeinbase(encrypted_this.get_mpz_t(), 2) + CHAR_BIT - 1) / CHAR_BIT;
        char* encrypted_this_buffer = new char[encrypted_this_size];
        mpz_export(encrypted_this_buffer, nullptr, 1, 1, 0, 0, encrypted_this.get_mpz_t());

        // 写入加密后的 "this"长度
        outFile.write(reinterpret_cast<const char*>(&encrypted_this_size), sizeof(size_t));

        // 写入加密后的 "this"
        outFile.write(encrypted_this_buffer, encrypted_this_size);

        // 写入压缩数据长度
        outFile.write(reinterpret_cast<const char*>(&size), sizeof(size_t));

        // 写入压缩数据本身
        outFile.write(buffer, size);

        if (DebugText == 1) {
            gmp_printf("compressed：%ZX\n", compressed.get_mpz_t()); // 输出十进制表示的 mpz_class 对象
            std::cout << "buffer (hex): ";
            for (size_t i = 0; i < size; ++i) {
                std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(static_cast<unsigned char>(buffer[i])); // 以十六进制形式输出每个字节
            }
        }

        // 释放内存
        delete[] buffer;
        delete[] encrypted_this_buffer;
    }

    outFile.close();
}

// 将加密数据从文件读取
vector<string> readAndDecompressData(const string& filename, const vector<mpz_class>& private_n, const vector<mpz_class>& private_d) {
    vector<string> decompressed_data;

    ifstream inFile(filename, ios::binary);
    if (!inFile) {
        cerr << "无法打开文件进行读取" << endl;
        return decompressed_data;
    }

    size_t _size;
    size_t size;
    while (inFile.read(reinterpret_cast<char*>(&_size), sizeof(size_t))) {
        char* cheac_this = new char[_size];
        inFile.read(cheac_this, _size);

        // 读取加密后的 "this"
        mpz_class encrypted_this;
        mpz_import(encrypted_this.get_mpz_t(), _size, 1, sizeof(char), 0, 0, cheac_this);
        if (DebugText == 1)gmp_printf("encrypted_this:%Zd\n", encrypted_this);
        //inFile.read(reinterpret_cast<char*>(encrypted_this.get_mpz_t()), encrypted_this.get_mpz_t()->_mp_alloc * sizeof(mp_limb_t));

        // 尝试解密 "this"
        mpz_class decrypted_this;
        bool decryption_successful = false;
        for (size_t i = 0; i < private_n.size(); ++i) {
            decrypt(decrypted_this, encrypted_this, private_n[i], private_d[i]);
            if (DebugText == 1)gmp_printf("decrypted_this:%Zd\n", decrypted_this);
            if (decrypted_this == mpz_class("74686973", 16)) { // "this" 的十六进制表示
                decryption_successful = true;
                break;
            }
        }

        // 如果解密成功，则继续解压缩数据
        if (decryption_successful) {
            // 读取压缩数据长度
            size_t size;
            inFile.read(reinterpret_cast<char*>(&size), sizeof(size_t));

            // 分配内存以存储压缩数据
            char* buffer = new char[size];

            // 读取压缩数据本身
            inFile.read(buffer, size);

            // 将读取的二进制数据转换为 mpz_class 对象
            mpz_class compressed;
            mpz_import(compressed.get_mpz_t(), size, 1, sizeof(char), 0, 0, buffer);
            if (DebugText == 1)gmp_printf("compressed：%ZX\n", compressed.get_mpz_t());

            // 释放内存
            delete[] buffer;

            // 解压缩数据
            string decompressed_data_piece = decompressString(compressed);
            cout << "decompressed_data_piece: " << decompressed_data_piece << endl;

            // 调整decompressed_data空间大小
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
        // 开始菜单
        chouse = 0;
        opening_menu();
        cin >> chouse;
        switch (chouse){
        
        //生成并保存一对私钥、密钥
        case 1: {
            string* KeyName = new string;
            cout << "请输入密钥名(将用于保存与识别)：";
            cin >> *KeyName;
            SaveKeys(*KeyName);

            //Debug
            if (DebugText == 1) {
                mpz_t n, e, d;
                mpz_inits(n, e, d, NULL);
                readMpzsFromFile(n, e, *KeyName + ".PublicKey");
                readMpzsFromFile(n, d, *KeyName + ".PrivateKey");
                gmp_printf("公钥: (%Zd, %Zd)\n", n, e);
                gmp_printf("私钥: (%Zd, %Zd)\n", n, d);
                mpz_clears(n, e, d, NULL);
            }

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
            string* texts = new string[text_quantity];

            // 读取公钥
            std::vector<PublicKeyInfo> publicKeys;

            // 遍历当前目录下的所有.PublicKey文件
            for (const auto& entry : filesystem::directory_iterator(".")) {
                if (entry.path().extension() == ".PublicKey") {
                    PublicKeyInfo keyInfo(entry.path().filename().string());

                    // 读取公钥
                    readMpzsFromFile(keyInfo.n, keyInfo.e, keyInfo.fileName);

                    // 将结构体添加到数组中
                    publicKeys.push_back(keyInfo);
                }
            }

            // 获取密钥与文本
            for (int i = 0;i < text_quantity;i++){
                system("cls");
                cout << "第" << i + 1 << "个：" << endl;
                cout << "请输入想选择的密钥：" << endl;
                // 输出所有读取到的公钥信息
                int n_keyInfo = 1;
                for (const auto& keyInfo : publicKeys) {
                    gmp_printf("文件名: %s\n公钥: (%Zd, %Zd)\n", keyInfo.fileName.c_str(), keyInfo.n, keyInfo.e);
                    //gmp_printf("第%d个-> 文件名: %s\n", n_keyInfo++, keyInfo.fileName.c_str());
                }
                cin >> password_chouse[i];
                cout << "请输入想加密的文本(按回车结束)：";
                cin.ignore(numeric_limits<streamsize>::max(), '\n'); // 清除输入流中的换行符
                getline(cin, texts[i]);
            }

            // 进行加密
            vector<vector<uint32_t>> allPlaintexts;
            for (int i = 0; i < text_quantity; i++) {
                mpz_t n, e;
                mpz_init(n);
                mpz_init(e);
                int check_key = 0;
                vector<uint32_t> plaintext = convertStringToNumbers(texts[i]);
                if (DebugText == 1) {
                    cout << "明文内容：";
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

            // 整合公钥
            // 将获取的公钥整合到 vector<mpz_class> 中
            vector<mpz_class> public_n;
            vector<mpz_class> public_e;
            for (int i = 0; i < text_quantity; i++) {
                if ((password_chouse[i] - 1) >= 0 && (password_chouse[i] - 1) < publicKeys.size()) { // 检查索引是否有效
                    // 将 mpz_t 转换为 mpz_class 类型
                    mpz_class mpz_n(publicKeys[(password_chouse[i] - 1)].n);
                    mpz_class mpz_e(publicKeys[(password_chouse[i] - 1)].e);

                    public_n.push_back(mpz_n);
                    public_e.push_back(mpz_e);
                }
                else {

                    cerr << "密钥选择无效：" << (password_chouse[i] - 1) << endl;
                }
            }

            // 写入数据到文件
            string save_filename;
            cout << "\n想要保存到的文件名：";
            cin >> save_filename;
            writeData(allPlaintexts, save_filename + ".TEPF", public_n, public_e);
            cout << "保存成功" << endl;
            system("pause");

            delete[] password_chouse, texts;

            break;
        }

        //寻找并解密一组文本
        case 3: {

            // 读取公钥
            std::vector<PrivateKeyInfo> privateKeys;

            // 遍历当前目录下的所有.PrivateKey文件
            for (const auto& entry : filesystem::directory_iterator(".")) {
                if (entry.path().extension() == ".PrivateKey") {
                    PrivateKeyInfo keyInfo(entry.path().filename().string());

                    // 读取公钥
                    readMpzsFromFile(keyInfo.n, keyInfo.d, keyInfo.fileName);

                    // 将结构体添加到数组中
                    privateKeys.push_back(keyInfo);
                }
            }

            // 获取密钥选择
            int password_chouse = 1;
            //system("cls");
            cout << "请输入想选择的密钥：" << endl;
            // 输出所有读取到的公钥信息
            int n_keyInfo = 1;
            for (const auto& keyInfo : privateKeys) {
                gmp_printf("文件名: %s\n公钥: (%Zd, %Zd)\n", keyInfo.fileName.c_str(), keyInfo.n, keyInfo.d);
                //gmp_printf("第%d个-> 文件名: %s\n", n_keyInfo++, keyInfo.fileName.c_str());
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

            // 创建包含单个元素的 vector
            std::vector<mpz_class> private_n_vector{ mpz_n };
            std::vector<mpz_class> private_d_vector{ mpz_d };

            // 从文件读取并解压缩数据
            vector<string> decompressedData = readAndDecompressData("1.TEPF", private_n_vector, private_d_vector);
            if (DebugText == 1) {
                cout << "decompressedData：";
                for (const auto& num : decompressedData) {
                    cout << num << " ";
                }
                cout << endl;
            }

            // 解密部分
            for (auto& str : decompressedData) {
                // 将 string 转换为 char* 数组
                const char* encryptedData = str.c_str();
                size_t encryptedSize = str.size();

                // 创建一个 mpz_class 对象并赋值
                mpz_class number;

                // 将二进制数据转换为 mpz_class 对象
                mpz_import(number.get_mpz_t(), encryptedSize, 1, 1, 1, 0, encryptedData);

                // 使用私钥进行解密
                mpz_class decryptedData;
                decrypt(decryptedData, number, mpz_n, mpz_d);

                // 将解密后的数据更新到原始字符串中
                str = mpz_get_str(nullptr, 16, decryptedData.get_mpz_t()); // 将 mpz_class 转换为 string
            }

            // 输出解压缩后的数据
            for (const auto& str : decompressedData) {
                cout << "结果：" << str << endl;
            }

            system("pause");
            break;
        }
        default:
            cout << "没有这个选项呦~\n";
            break;
        }
    }
    /*
    string words;
    cout << "请输入要加密的字符串：";
    getline(cin, words);

    mpz_t n, e, d;
    generateRSAKeys(n, e, d);

    // 打印公钥和私钥
    gmp_printf("公钥: (%Zd, %Zd)\n", n, e);
    gmp_printf("私钥: (%Zd, %Zd)\n", n, d);

    // 将字符串转换为数字数组
    vector<uint32_t> plaintext = convertStringToNumbers(words);

    // 打印数字数组的十六进制形式
    printNumbersAsHex(plaintext);

    // 加密数字数组并打印加密结果
    encryptAndPrint(plaintext, n, e);

    // 将压缩后的字符串写入文件
    compressAndWriteToFile(words);

    // 从文件中读取压缩后的字符串并解压缩
    string decompressed = readAndDecompressFromFile();
    cout << "解压后的字符串：" << decompressed << endl;

    mpz_clears(n, e, d, NULL);
    return 0;
    */
}
