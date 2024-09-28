#include "FixedFloat.hpp"

#include <stdexcept>
#include <functional>
#include <iostream>

uint32_t FixedFloat::read_n_bit(uint16_t i, uint16_t n) const {
    if (n > 32) n = 32; // 位数不超过 32
    if (i >= this->bit_len) return 0; // 超过最大位数
    uint16_t h = i >> 5; // 32 位的 uint32_t 数组下标
    uint16_t l = i & 31; // 32 位的 uint32_t 中的第几位
    uint32_t ret = 0;
    if (l + n <= 32) { // 位数不跨两个 uint32_t
        ret = (this->arr[h] >> l) & (((uint64_t) 1 << n) - 1);
    } else { // 位数跨两个 uint32_t
        ret = (this->arr[h] >> l) & (((uint64_t) 1 << (32 - l)) - 1);
        ret |= (this->arr[h + 1] & ((uint64_t) (1 << (n + l - 32)) - 1)) << (32 - l);
    }
    return ret;
}
uint32_t FixedFloat::write_n_bit(uint16_t i, uint16_t n, uint32_t val) {
    if (n > 32) n = 32; // 位数不超过 32
    if (i >= this->bit_len) return 0; // 超过最大位数
    val &= ((uint64_t) 1 << n) - 1; // 只取 n 位
    uint16_t h = i >> 5; // 32 位的 uint32_t 数组下标
    uint16_t l = i & 31; // 32 位的 uint32_t 中的第几位
    if (l + n <= 32) { // 位数不跨两个 uint32_t
        this->arr[h] &= ~((((uint64_t) 1 << n) - 1) << l);
        this->arr[h] |= val << l;
    } else { // 位数跨两个 uint32_t
        this->arr[h] &= ((uint64_t) 1 << l) - 1;
        this->arr[h] |= val << l;
        this->arr[h + 1] &= ~(((uint64_t) 1 << (n + l - 32)) - 1);
        this->arr[h + 1] |= val >> (32 - l);
    }
    return val;
}
uint32_t FixedFloat::read_digit(uint16_t i) const {
    return read_n_bit(i * bit_per_digit, bit_per_digit); // 读取第 i 位数字
}
uint32_t FixedFloat::write_digit(uint16_t i, uint32_t val) {
    return write_n_bit(i * bit_per_digit, bit_per_digit, val); // 写入第 i 位数字
}
uint16_t FixedFloat::least_significant_bit() const {
    uint16_t i = 0;
    for (i = 0; i < len; i++) // 找到第一个非零的 uint32_t
        if (arr[i]) break;
    if (i == len) return bit_len; // 如果都是 0，则返回最大位数
    for (uint16_t j = i * 32; j < (i + 1) * 32; j++) // 找到第一个非零的 bit
        if (read_n_bit(j, 1))
            return j;
    return bit_len;
}
uint16_t FixedFloat::least_significant_digit() const{
    return least_significant_bit() / bit_per_digit; // 最低有效数字，即最低有效位对应的数字
}
void FixedFloat::clear_int() {
    for (uint16_t i = 0; i < int_digit_len; i++) {
        write_digit(dec_digit_len + i, 0);
    }
}
void FixedFloat::clear_dec() {
    for (uint16_t i = 0; i < dec_digit_len; i++) {
        write_digit(i, 0);
    }
}
bool FixedFloat::is_zero() const{
    return least_significant_bit() == bit_len; // 最低有效位为最大位数，则为 0
}

FixedFloat::FixedFloat(uint16_t base, uint16_t int_digit_len, uint16_t dec_digit_len): 
                        base(base),
                        int_digit_len(int_digit_len),
                        dec_digit_len(dec_digit_len), 
                        bit_per_digit(ceil(log2(base))), // 根据基数计算每个数字占用的最低 bit 数
                        bit_len((int_digit_len + dec_digit_len) * bit_per_digit),
                        len((bit_len + 31) >> 5) {
    this->arr = (uint32_t *) calloc(this->len, 32);
}
FixedFloat::FixedFloat(uint16_t base, uint16_t int_digit_len, uint16_t dec_digit_len, double d): FixedFloat(base, int_digit_len, dec_digit_len) {
    // 输入的数字超过了最大值或最小值，则取最大值或最小值
    const double MAX_VALUE = pow(base, int_digit_len) - 1;
    if (d > MAX_VALUE) d = MAX_VALUE;
    else if (d < -MAX_VALUE) d = -MAX_VALUE;

    // 判断符号
    if (d < 0) {
        sign = true;
        d = -d;
    }

    // write integer part
    uint32_t i = (uint32_t) d;
    d -= i;
    for (uint16_t j = 0; j < int_digit_len && i; j++) {
        write_digit(dec_digit_len + j, i % base);
        i /= base;
    }

    // write decimal part
    for (uint16_t j = 0; j < dec_digit_len && d; j++) {
        d *= base;
        i = (uint32_t) d;
        write_digit(dec_digit_len - j - 1, i);
        d -= i;
    }

    if (is_zero()) sign = false; // if all digits are 0, then it is positive
}

FixedFloat::FixedFloat(uint16_t base, uint16_t int_digit_len, uint16_t dec_digit_len, const std::string &str): FixedFloat(base, int_digit_len, dec_digit_len) {
    if (base > 36) throw std::runtime_error("base should be less than 36");
    int16_t i = 0; // i is the first number character
    uint32_t size = str.size();
    const std::function<uint32_t(char)> ctoi = [] (char c) {
        return c >= '0' && c <= '9' ? c - '0' : c >= 'a' && c <= 'z' ? c - 'a' + 10 : c - 'A' + 10;
    };
    
    // trim leading and trailing spaces
    while (i < size && str[i] == ' ') i++;
    while (size > 0 && str[size - 1] == ' ') size--;

    // judge sign
    if (str[i] == '-') {
        sign = true;
        i++;
    } else if (str[i] == '+') {
        i++;
    }
    // skip leading zeros
    while (str[i] == '0' && i < size) i++;
    // if all 0 or empty then return
    if (i == size) return;

    // find decimal point
    size_t dot = str.find('.');
    // if not found
    if (dot == std::string::npos) 
        dot = size;
    // parse integer part
    for (int16_t j = 0; j < dot - i && j < int_digit_len; j++) {
        write_digit(dec_digit_len + j, ctoi(str[dot - 1 - j]));
    }

    // if there isn't decimal point then return
    if (dot == size) return;
    // parse decimal part
    for (int16_t j = 0; j < size - dot - 1 && j < dec_digit_len; j++) {
        write_digit(dec_digit_len - j - 1, ctoi(str[dot + 1 + j]));
    }

    if (is_zero()) sign = false; // if all digits are 0, then it is positive
}

// copy constructor
FixedFloat::FixedFloat(const FixedFloat &f) {
    this->sign = f.sign;
    this->base = f.base;
    this->int_digit_len = f.int_digit_len;
    this->dec_digit_len = f.dec_digit_len;
    this->bit_per_digit = f.bit_per_digit;
    this->bit_len = f.bit_len;
    this->len = f.len;
    this->arr = (uint32_t *) calloc(this->len, 32);
    for (uint16_t i = 0; i < len; i++) {
        this->arr[i] = f.arr[i];
    }
}

FixedFloat& FixedFloat::operator=(const FixedFloat &f) {
    if (this == &f) return *this;
    if (arr) free(arr);
    this->sign = f.sign;
    this->base = f.base;
    this->int_digit_len = f.int_digit_len;
    this->dec_digit_len = f.dec_digit_len;
    this->bit_per_digit = f.bit_per_digit;
    this->bit_len = f.bit_len;
    this->len = f.len;
    this->arr = (uint32_t *) calloc(this->len, 32);
    for (uint16_t i = 0; i < len; i++) {
        this->arr[i] = f.arr[i];
    }
    return *this;
}

// move constructor
FixedFloat::FixedFloat(FixedFloat &&f) {
    this->sign = f.sign;
    this->base = f.base;
    this->int_digit_len = f.int_digit_len;
    this->dec_digit_len = f.dec_digit_len;
    this->bit_per_digit = f.bit_per_digit;
    this->bit_len = f.bit_len;
    this->len = f.len;
    auto arr =  f.arr;
    f.arr = nullptr;
    this->arr = arr;
}

FixedFloat::~FixedFloat() {
    if (arr) free(arr);
}

int32_t FixedFloat::intValue() const {
    double d = 0;
    int32_t ret = 0;
    // integer part
    for (uint16_t i = int_digit_len + dec_digit_len - 1; i >= dec_digit_len; i--) {
        d *= base;
        d += read_digit(i);
        if (d > INT32_MAX) break;
    }
    ret = d > INT32_MAX ? INT32_MAX : (int32_t) d; // if overflow, return INT32_MAX
    return sign ? -d : d;
}

double FixedFloat::doubleValue() const {
    double int_ret = 0;
    // integer part
    for (uint16_t i = int_digit_len + dec_digit_len - 1; i >= dec_digit_len; i--) {
        int_ret *= base;
        int_ret += read_digit(i);
    }
    double dec_ret = 0;
    // decimal part
    uint16_t lsd = least_significant_digit();
    for (uint16_t i = lsd; i < dec_digit_len; i++) {
        dec_ret /= base;
        dec_ret += read_digit(i);
    }
    dec_ret /= base;
    double ret = int_ret + dec_ret;
    if (sign) ret = -ret;
    return ret;
}

FixedFloat FixedFloat::operator-() const{
    FixedFloat ret(*this);
    ret.sign = !this->sign;
    return ret;
}

bool FixedFloat::operator>(const FixedFloat& other) const {
    return !(*this < other || *this == other);
}

bool FixedFloat::operator==(const FixedFloat& other) const {
    // 如果基数或者整数部分位数或者小数部分位数不同，则无法比较
    if (base != other.base || int_digit_len != other.int_digit_len || dec_digit_len != other.dec_digit_len)
        throw std::runtime_error("can not compare two FixedFloat with different base or length");
    if (this->sign != other.sign) return false;
    // 从最低有效数字开始比较
    uint16_t lsd1 = least_significant_digit();
    uint16_t lsd2 = other.least_significant_digit();
    // 如果最低有效数字不同，则两数不相等
    if (lsd1 != lsd2) return false;
    uint16_t end = lsd1 < lsd2 ? lsd1 : lsd2;
    // 读取每一位数字进行比较
    for (uint16_t i = int_digit_len + dec_digit_len - 1; i >= end; i--) {
        if (read_digit(i) != other.read_digit(i)) return false;
    }
    return true;
}

bool FixedFloat::operator<(const FixedFloat& other) const {
    // 如果基数或者整数部分位数或者小数部分位数不同，则无法比较
    if (base != other.base || int_digit_len != other.int_digit_len || dec_digit_len != other.dec_digit_len)
        throw std::runtime_error("can not compare two FixedFloat with different base or length");
    // 如果当前数是负数，而另一个数是正数，则当前数小于另一个数
    if (this->sign && !other.sign)
        return true;
    // 如果当前数是正数，而另一个数是负数，则当前数大于另一个数
    else if (!this->sign && other.sign)
        return false;
    uint16_t lsd1 = least_significant_digit();
    uint16_t lsd2 = other.least_significant_digit();
    // 从最低有效数字开始比较
    uint16_t end = lsd1 < lsd2 ? lsd1 : lsd2;
    // 读取每一位数字进行比较
    for (uint16_t i = int_digit_len + dec_digit_len - 1; i >= end; i--) {
        if (read_digit(i) > other.read_digit(i)) {
            if (this->sign) return true;
            else return false;
        } else if (read_digit(i) < other.read_digit(i)) {
            if (this->sign) return false;
            else return true;
        }
    }
    return false;
}

FixedFloat FixedFloat::operator+(const FixedFloat& other) const {
    // 如果基数或者整数部分位数或者小数部分位数不同，则无法相加
    if (base != other.base || int_digit_len != other.int_digit_len || dec_digit_len != other.dec_digit_len)
        throw std::runtime_error("can not add two FixedFloat with different base or length");
    FixedFloat ret(base, int_digit_len, dec_digit_len);
    // 如果两数符号不同，则转化为减法
    if (this->sign != other.sign) {
        return *this - (-other);
    }
    ret.sign = this->sign;
    uint16_t lsd1 = least_significant_digit();
    uint16_t lsd2 = other.least_significant_digit();
    uint16_t i = lsd1 < lsd2 ? lsd1 : lsd2;
    uint32_t carry = 0;
    for (; i < int_digit_len + dec_digit_len; i++) {
        int32_t val = read_digit(i) + other.read_digit(i) + carry;
        if (val >= base) {
            val -= base;
            carry = 1;
        } else {
            carry = 0;
        }
        ret.write_digit(i, val);
    }
    return ret;
}

FixedFloat FixedFloat::operator-(const FixedFloat& other) const {
    // 如果基数或者整数部分位数或者小数部分位数不同，则无法相减
    if (base != other.base || int_digit_len != other.int_digit_len || dec_digit_len != other.dec_digit_len)
        throw std::runtime_error("can not add two FixedFloat with different base or length");
    FixedFloat ret(base, int_digit_len, dec_digit_len);
    // 如果两数符号不同，则转化为加法
    if (this->sign != other.sign) {
        return *this + (-other);
    }

    // if both are zero, then return according to the sign
    if (is_zero()) return -other;
    else if (other.is_zero()) return *this;

    // if this >= other and this is negative, then return -(other - this)
    // else if this < other and this is positive, then return -(other - this)
    if (*this < other != this->sign) {
        return -(other - *this);
    }
    
    ret.sign = this->sign;
    uint16_t lsd1 = least_significant_digit();
    uint16_t lsd2 = other.least_significant_digit();
    uint16_t i = lsd1 < lsd2 ? lsd1 : lsd2;
    uint32_t carry = 0;
    for (; i < int_digit_len + dec_digit_len; i++) {
        int32_t val = read_digit(i) - other.read_digit(i) - carry;
        if (val < 0) {
            val += base;
            carry = 1;
        } else {
            carry = 0;
        }
        ret.write_digit(i, val);
    }
    return ret;
}

FixedFloat FixedFloat::operator*(const FixedFloat& other) const {
    // 如果基数或者整数部分位数或者小数部分位数不同，则无法相乘
    if (base != other.base || int_digit_len != other.int_digit_len || dec_digit_len != other.dec_digit_len)
        throw std::runtime_error("can not add two FixedFloat with different base or length");
    FixedFloat ret(base, int_digit_len, dec_digit_len);
    ret.sign = this->sign ^ other.sign;
    uint16_t lsd1 = least_significant_digit();
    uint16_t lsd2 = other.least_significant_digit();
    // 从最低有效数字开始相乘
    int16_t i = lsd1 < lsd2 ? lsd1 : lsd2;
    // 对于每一位数字，计算乘积并加到 ret 上
    for (; i < int_digit_len + dec_digit_len; i++) {
        uint32_t carry = 0;
        for (int16_t j = lsd2 < dec_digit_len - i ? lsd2 : dec_digit_len - i; j < int_digit_len + dec_digit_len; j++) {
            uint32_t val = read_digit(i) * other.read_digit(j) + carry + ret.read_digit(i + j - dec_digit_len);
            carry = val / base;
            val %= base;
            ret.write_digit(i + j - dec_digit_len, val);
        }
    }
    return ret;
}

std::string FixedFloat::toString() const {
    std::string ret = "";
    if (sign) ret += "-";
    bool flag = false; // flag to indicate if there is any non-zero digit
    const std::function<char(uint32_t)> itoc = [] (uint32_t i) {
        return i < 10 ? '0' + i : 'A' + i - 10;
    };
    // integer part
    for (int16_t i = int_digit_len + dec_digit_len - 1; i >= dec_digit_len; i--) {
        uint32_t val = read_digit(i);
        if (!val && !flag) continue;
        ret += itoc(val);
        flag = true;
    }
    if (!flag) ret += "0";
    flag = false;
    ret += ".";
    // store decimal by least significant digit
    std::string tmp = ret;
    for (int16_t i = dec_digit_len - 1 ; i >= 0; i--) {
        uint32_t val = read_digit(i);
        tmp += itoc(val);
        if (val) {  // if there is any non-zero digit, then update ret
            ret = tmp;
            flag = true;
        }
    }
    if (!flag) ret += "0"; // if all digits are zero, then add a zero
    return ret;
}

FixedFloat FixedFloat::baseTo(uint16_t base) const{
    return this->convertTo(base, this->int_digit_len, this->dec_digit_len);
}

FixedFloat FixedFloat::convertTo(uint16_t base, uint16_t int_digit_len, uint16_t dec_digit_len) const {
    FixedFloat ret(base, int_digit_len, dec_digit_len);

    // if base, int_digit_len, dec_digit_len are the same, then return itself
    if (base == this->base && int_digit_len == this->int_digit_len && dec_digit_len == this->dec_digit_len) {
        ret = *this;
        return ret;
    }

    FixedFloat tmp(*this), mul(base, int_digit_len, dec_digit_len, this->base), mul2(this->base, this->int_digit_len, this->dec_digit_len, base);
    tmp.sign = false;

    // calculate the most significant bit
    uint16_t msb = this->dec_digit_len;
    for (int16_t i = this->int_digit_len + this->dec_digit_len - 1; i >= this->dec_digit_len; i--)
        if (read_digit(i)) {
            msb = i;
            break;
        }
    // from the most significant digit to unit digit, update the return value to get the converted integer part
    for (; msb >= this->dec_digit_len; msb--) {
        ret = ret * mul + FixedFloat(base, int_digit_len, dec_digit_len, read_digit(msb));
    }

    tmp = *this;
    tmp.sign = false;

    // from unit digit to the last digit, update the return value to get the converted decimal part
    for (uint16_t i = 1; i <= dec_digit_len; i++) {
        tmp.clear_int();
        tmp = tmp * mul2;
        int32_t val = tmp.intValue();
        ret.write_digit(dec_digit_len - i,  val % base);
    }

    // copy the sign
    ret.sign = this->sign;

    return ret;
}
