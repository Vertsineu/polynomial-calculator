# polynomial-calculator
High-precision positive integer degree polynomial calculator.

It can be used to calculate the value of one-variable integer degree polynomial, e.g. the value of `3/7x^2-1/3x+2` when `x = 1.4` with high precision.

It can also be used to change the base and precision among different high-precision number, e.g. `523.43` in decimal to `20B.6E1...` in hexadecimal when `int_digit_len` is set to `20` and `dec_digit_len` is set to `200`.

Use the command below to compile the project:
```shell
g++ main.cpp FixedFloat.cpp Expression.cpp -o main
```
And run it by the command below:
```shell
./main
```
