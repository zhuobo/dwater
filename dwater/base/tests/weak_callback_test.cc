// Author:          Drinkwater
// Email:           tanzhuobo@gmail.com
// Last modified:   2021.03.25
// Filename:        weak_callback_test.cc 
// Descripton:       

#include "dwater/base/weak_callback.h"
#include <functional>
#include <iostream>


struct Foo
{
public:
    Foo(int num) : num_(num) {}

    void printAdd(int a) {
        std::cout << "==printAdd: " << (num_ + a) << std::endl;
    }
private:
    int num_;
};

int main() {
    std::shared_ptr<Foo> ptr_foo(new Foo(5));
    auto wcb = dwater::MakeWeakCallback(ptr_foo, &Foo::printAdd);
    wcb(5);
    return 0;
}

