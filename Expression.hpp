#ifndef __EXPRESSION_HPP__
#define __EXPRESSION_HPP__

#include <cstdint>
#include <cmath>
#include <stack>
#include <queue>
#include <stdexcept>
#include <functional>
#include "FixedFloat.hpp"

class Expression {
    private:
        // 经过解析后的后缀表达式
        std::queue<std::string> expr;
        // 获取运算符的优先级，比如 + - 为 1，* / 为 2，^ 为 3，优先级越高越先计算
        uint32_t get_priority(const char c) const;
        // 判断字符是否为数字
        bool is_num(const char c) const;
        // 将字符转换为数字
        uint32_t get_num(const char c) const;
        // 判断字符是否为小数点
        bool is_dot(const char c) const;
        // 判断字符是否为变量
        bool is_variable(const char c) const;
        // 判断字符是否为运算符
        bool is_op(const char c) const;
        // 判断字符是否为左括号
        bool is_left_bracket(const char c) const;
        // 判断字符是否为右括号
        bool is_right_bracket(const char c) const;
    public:
        // 通过输入的中缀表达式构造一个 Expression
        Expression(const std::string& input);
        // 给定一个 x，计算表达式的值
        FixedFloat eval(FixedFloat x) const;
};

#endif