#include <cstdint>
#include <cmath>
#include <iostream>
#include <stack>
#include <queue>
#include <functional>
#include <stdexcept>
#include "Expression.hpp"

int main() {
    while (true) {
        std::cout << "Please choose a mode (1 to base conversion, 2 to expression evaluation, q to quit): ";
        std::string mode_str;
        std::getline(std::cin, mode_str);
        if (mode_str == "q") break;
        if (mode_str != "1" && mode_str != "2") {
            std::cout << "Invalid mode" << std::endl;
            continue;
        }

        uint16_t mode = std::stoi(mode_str);

        switch (mode) {
            case 1: {
                
                    std::cout << "Please choose a base: ";
                    std::string base_str;
                    std::getline(std::cin, base_str);
                    uint16_t base = std::stoi(base_str);

                    std::cout << "Please choose an integer digit length: ";
                    std::string int_digit_len_str;
                    std::getline(std::cin, int_digit_len_str);
                    uint16_t int_digit_len = std::stoi(int_digit_len_str);

                    std::cout << "Please choose a decimal digit length: ";
                    std::string dec_digit_len_str;
                    std::getline(std::cin, dec_digit_len_str);
                    uint16_t dec_digit_len = std::stoi(dec_digit_len_str);

                    std::cout << "Please input a number: ";
                    std::string num_str;
                    std::getline(std::cin, num_str);
                    FixedFloat f(base, int_digit_len, dec_digit_len, num_str);

                while (true) {
                    std::cout << "Please choose a base to convert to (q to quit): ";
                    std::string base_to_str;
                    std::getline(std::cin, base_to_str);
                    if (base_to_str == "q") break;
                    uint16_t base_to = std::stoi(base_to_str);

                    FixedFloat ret = f.baseTo(base_to);

                    std::cout << "The result is: " << ret.toString() << std::endl;
                }
                break;
            }
            case 2: {
                std::cout << "Please input an expression: ";
                std::string expr;
                std::getline(std::cin, expr);
                try {
                    Expression e(expr);
                    while (true) {
                        std::cout << "Please input a value for x (q to quit): ";
                        std::string input;
                        std::getline(std::cin, input);
                        if (input == "q") break;
                        FixedFloat x(10, 20, 200, input);
                        std::string result = e.eval(x).toString();
                        std::cout << "The result is: " << result << std::endl;
                    }
                } catch (std::runtime_error &e) {
                    std::cout << "Invalid: " << e.what() << std::endl;
                    continue; // continue to input
                }
                break;
            }
        }

        
    }
}