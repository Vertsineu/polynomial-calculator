#include <cstdint>
#include <cmath>
#include <stack>
#include <queue>
#include <stdexcept>
#include <functional>
#include "FixedFloat.hpp"
#include "Expression.hpp"


uint32_t Expression::get_priority(const char c) const {
    switch (c) {
        case '+':
        case '-':
            return 1;
        case '*':
        case '/':
            return 2;
        case '^':
            return 3;
        default:
            return 0;
    }
}
bool Expression::is_num(const char c) const {
    return c >= '0' && c <= '9';
}
uint32_t Expression::get_num(const char c) const {
    return c - '0';
}
bool Expression::is_dot(const char c) const {
    return c == '.';
}
bool Expression::is_variable(const char c) const {
    return c == 'x';
}
bool Expression::is_op(const char c) const {
    return c == '+' || c == '-' || c == '*' || c == '/' || c == '^';
}
bool Expression::is_left_bracket(const char c) const {
    return c == '(';
}
bool Expression::is_right_bracket(const char c) const {
    return c == ')';
}
Expression::Expression(const std::string& input) {
    const uint32_t len = input.length();
    std::string num_str;
    std::stack<char> op_stack;
    bool add_mul = false; // add '*' operator when necessary, e.g. 2x -> 2*x
    bool add_zero = false; // add '0' when necessary, e.g. -1/2 -> 0-1/2, +3 -> 0+3
    char last_c = '('; // last effective character
    for (uint32_t i = 0; i < len; i++) {
        char c = input[i];

        // c = 'x' or '('
        if (is_variable(c) || is_left_bracket(c)) { 
            // last_c = 'x' or  ')' or '0'-'9' or '.'
            if (is_variable(last_c) || is_right_bracket(last_c) || is_num(last_c) || is_dot(last_c)) {
                add_mul = true;
            }
        // c = '0'-'9' or '.'
        } else if (is_num(c) || is_dot(c)) {
            // last_c = '0'-'9' or '.'
            if (is_right_bracket(last_c) || is_variable(last_c)) {
                add_mul = true;
            }
        }

        // c = '-' or '+'
        if (c == '-' || c == '+') {
            // last_c = '('
            if (is_left_bracket(last_c)) {
                add_zero = true;
            }
        }

        if (add_mul) { // allow leaving out the '*' operator
            c = '*';
            last_c = c;
            add_mul = false;
            i--;
        }

        if (add_zero) { // allow input +3 or -1/2
            c = '0';
            last_c = c;
            add_zero = false;
            i--;
        }

        // if c is a part of a number, then parse it
        if (is_num(c) || is_dot(c)) {
            num_str.push_back(c);

            last_c = c;
            continue;
        } 
        // if num_str is not empty, it means that the number has ended, then push it to the queue
        else if (!num_str.empty()) { 
            expr.push(num_str);
            num_str.clear();
            // no continue here
        }

        // if c is a left bracket, then push it to the stack
        if (is_left_bracket(c)) {
            op_stack.push(c);

            last_c = c;
            continue;
        } 
        // if c is a right bracket, then pop all operators from the stack until a left bracket is encountered
        else if (is_right_bracket(c)) {
            while (!op_stack.empty() && op_stack.top() != '(') {
                expr.push(std::string(1, op_stack.top()));
                op_stack.pop();
            }
            if (!op_stack.empty()) {
                op_stack.pop();
            }

            last_c = c;
            continue;
        }

        // if c is a variable, then push it to the queue
        if (is_variable(c)) {
            expr.push("x");

            last_c = c;
            continue;
        }

        // if c is an operator, then pop all operators from the stack whose priority is not less than c and push it to the queue
        if (is_op(c)) {
            while (!op_stack.empty() && get_priority(op_stack.top()) >= get_priority(c)) {
                expr.push(std::string(1, op_stack.top()));
                op_stack.pop();
            }
            op_stack.push(c);

            last_c = c;
            continue;
        }
        
    }

    // clear the remaining number
    if (!num_str.empty()) {
        expr.push(num_str);
    }
    // clear the remaining operators
    while (!op_stack.empty()) {
        expr.push(std::string(1, op_stack.top()));
        op_stack.pop();
    }
}
FixedFloat Expression::eval(FixedFloat x) const {
    std::stack<FixedFloat> stack;
    std::queue<std::string> expr = this->expr;
    // parse postfix expression   
    while (!expr.empty()) {
        std::string ele = expr.front();
        expr.pop();
        switch (ele[0]) {
            case '+': {
                FixedFloat a = stack.top();
                stack.pop();
                FixedFloat b = stack.top();
                stack.pop();
                stack.push(a + b);
                break;
            }
            case '-': {
                FixedFloat a = stack.top();
                stack.pop();
                FixedFloat b = stack.top();
                stack.pop();
                stack.push(b - a);
                break;
            }
            case '*': {
                FixedFloat a = stack.top();
                stack.pop();
                FixedFloat b = stack.top();
                stack.pop();
                stack.push(a * b);
                break;
            }
            case '/': {
                FixedFloat a = stack.top();
                stack.pop();
                FixedFloat b = stack.top();
                stack.pop();
                stack.push(FixedFloat(10, x.int_digit_len, x.dec_digit_len, b.doubleValue() / a.doubleValue()));
                break;
            }
            case '^': {
                FixedFloat a = stack.top();
                stack.pop();
                FixedFloat b = stack.top();
                stack.pop();
                FixedFloat c = FixedFloat(10, x.int_digit_len, x.dec_digit_len, 1);
                for (int32_t i = 0; i < a.intValue(); i++) {
                    c = c * b;
                }
                stack.push(c);
                break;
            }
            case 'x': {
                stack.push(x);
                break;
            }
            // if it is a number
            default: {
                stack.push(FixedFloat(10, x.int_digit_len, x.dec_digit_len, ele));
                break;
            }
        
        }
    }
    if (stack.size() != 1) {
        throw std::runtime_error("invalid expression");
    }
    return stack.top();
}