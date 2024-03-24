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

// 一个函数用于使用RSA解密一条消息
void decrypt(mpz_t m, mpz_t c, mpz_t n, mpz_t d) {
    // 计算m = c^d mod n
    mod_pow(m, c, d, n); // 使用gmp提供的模幂函数
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

int main() {
    string words;
    cout << "请输入要加密的字符串：";
    getline(cin, words);

    vector<uint32_t> plaintext;
    for (size_t i = 0; i < words.size(); ++i) {
        unsigned char c = words[i];
        plaintext.push_back((uint32_t)c);
    }

    cout << "UTF-8 编码转换为数字：";
    for (uint32_t num : plaintext) {
        cout << hex << num << " ";
    }
    cout << endl;

    mpz_t n, e, d;
    mpz_inits(n, e, d, NULL);
    keygen(n, e, d);

    gmp_printf("公钥: (%Zd, %Zd)\n", n, e);
    gmp_printf("私钥: (%Zd, %Zd)\n", n, d);

    const int block_size = mpz_sizeinbase(n, 2) / 8 - 1;
    cout << "每个小块的长度: " << block_size << " 字节" << endl;

    mpz_t ciphertext;
    mpz_init(ciphertext);

    cout << "加密结果：";
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

    // 将压缩后的字符串写入文件
    mpz_class compressed = compressString(words);
    writeBigIntToFile(compressed, "Test.TEPF");

    // 从文件中读取压缩后的字符串
    mpz_class read_compressed = readBigIntFromFile("Test.TEPF");
    std::string decompressed = decompressString(read_compressed);
    cout << "解压后的字符串：" << decompressed << endl;

    mpz_clears(n, e, d, ciphertext, NULL);
    return 0;
}
