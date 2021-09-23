#include <iostream>
#include <derivative_engine.h>
#include <tmath.h>

int main()
{
    term::variable x;
    term::variable exp = term::exp(x);
    term::variable C = 5.0;

    term::variable a = x ^ x;
    term::variable b = (2 * C) ^ exp;
    term::variable c = term::sin(x) ^ term::cos(x);
    term::variable d = x * exp;

    term::custom fun = { std::sin, std::cos };

    std::cout << std::fixed;
    std::cout << fun(std::acos(-1)) << std::endl;

    std::cout << a(2.0) << std::endl;
    std::cout << b(-0.5) << std::endl;
    std::cout << c(2.0) << std::endl;
    std::cout << d(3.5) << std::endl;

    term::variable y;
    term::variable exp_y = term::exp(y);
    std::cout << exp_y(1.0) << std::endl;
    y = -(y ^ 2);
    std::cout << y(1.0) << std::endl;
}