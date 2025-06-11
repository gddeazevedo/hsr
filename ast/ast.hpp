#ifndef __ast
#define __ast

#include <iostream>
#include <string>
#include <memory>
#include <cctype>


struct Expr {
    virtual ~Expr() = default;
    virtual void print(int depth = 0) const = 0;
};

struct Number : Expr {
    double value;
    Number(double v) : value(v) {}

    void print(int depth = 0) const override {
        std::cout << std::string(depth, ' ') << "Number: " << value << std::endl;
    }
};

struct Binary : Expr {
    char op;
    std::unique_ptr<Expr> left;
    std::unique_ptr<Expr> right;

    Binary(char op, std::unique_ptr<Expr> l, std::unique_ptr<Expr> r)
        : op(op), left(std::move(l)), right(std::move(r)) {}

    void print(int depth = 0) const override {
        std::cout << std::string(depth, ' ') << "Binary(" << op << ")\n";
        left->print(depth + 2);
        right->print(depth + 2);
    }
};


#endif