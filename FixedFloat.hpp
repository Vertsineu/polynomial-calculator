#ifndef __FIXED_FLOAT_HPP__
#define __FIXED_FLOAT_HPP__

#include <cstdint>
#include <cstdlib>
#include <cmath>
#include <string>

class FixedFloat {
    private:
        friend class Expression;
        bool sign = false; // 符号位
        uint16_t base; // 基数
        uint16_t int_digit_len; // 整数部分的位数
        uint16_t dec_digit_len; // base^(-dec_digit_len) 为实际精度，dec_digit_len就是小数点后的位数

        uint16_t bit_per_digit; // 每个数字占用的 bit 数
        uint16_t bit_len; // 整个数字占用的 bit 数

        uint16_t len; // 所分配的数组的长度
        uint32_t *arr = nullptr; // 存储的数组

        uint32_t read_n_bit(uint16_t i, uint16_t n) const;          // 从 arr 中读取从 i 开始的 n bit 位
        uint32_t write_n_bit(uint16_t i, uint16_t n, uint32_t val); // 从 arr 中写入从 i 开始的 n bit 位
        uint32_t read_digit(uint16_t i) const;                      // 读取第 i 位数字
        uint32_t write_digit(uint16_t i, uint32_t val);             // 写入第 i 位数字
        uint16_t least_significant_bit() const;                     // 最低有效位，即数组中最低的非零 bit 位
        uint16_t least_significant_digit() const;                   // 最低有效数字，即最低有效位对应的数字
        void clear_int();                                           // 清空整数部分
        void clear_dec();                                           // 清空小数部分
        bool is_zero() const;                                             // 判断是否为 0
    public:
        // 通过基数，整数部分位数，小数部分位数构造一个 FixedFloat，并初始化可能的初值
        FixedFloat(uint16_t base, uint16_t int_digit_len, uint16_t dec_digit_len);
        FixedFloat(uint16_t base, uint16_t int_digit_len, uint16_t dec_digit_len, double d);
        FixedFloat(uint16_t base, uint16_t int_digit_len, uint16_t dec_digit_len, const std::string &str);
        // 拷贝构造函数
        FixedFloat(const FixedFloat &f);
        // 赋值构造函数
        FixedFloat& operator=(const FixedFloat &f);
        // 移动构造函数
        FixedFloat(FixedFloat &&f);
        // 析构函数
        ~FixedFloat();
        // 返回整数值
        int32_t intValue() const;
        // 返回浮点数值
        double doubleValue() const;
        // 取负数
        FixedFloat operator-() const;
        // 判断是否大于
        bool operator>(const FixedFloat& other) const;
        // 判断是否等于
        bool operator==(const FixedFloat& other) const;
        // 判断是否小于
        bool operator<(const FixedFloat& other) const;
        // 两数相加
        FixedFloat operator+(const FixedFloat& other) const;
        // 两数相减
        FixedFloat operator-(const FixedFloat& other) const;
        // 两数相乘
        FixedFloat operator*(const FixedFloat& other) const;
        // 获得该数的字符串表示
        std::string toString() const;
        // 将该数转换为指定基数
        FixedFloat baseTo(uint16_t base) const;
        // 将该数转换为指定基数，整数部分位数，小数部分位数
        FixedFloat convertTo(uint16_t base, uint16_t int_digit_len, uint16_t dec_digit_len) const;
};

#endif