// Author:          Drinkwater
// Email:           tanzhuobo@gmail.com
// Last modified:   2021.03.20
// Filename:        singleton_test.cc
// Descripton:      testing singleton 

#include "../singleton.h"
#include "../current_thread.h"
#include "../thread.h"

class Test : dwater::noncopyable {
public:
    Test() {
        printf("tid = %d, constructing %p\n", dwater::current_thread::Tid(), this);
    }
    ~Test() {
        printf("tid = %d, constructing %p %s\n", dwater::current_thread::Tid(), this, name_.c_str());
    }

    const dwater::string& Name() const {
        return name_;
    }

    void SetName(const dwater::string& str) {
        name_ = str;
    }

private:
    dwater::string name_;
};

class TestNoDestroy : dwater ::noncopyable {
public:
    void no_destroy();

    TestNoDestroy() {
        printf("tid = %d, constructing TestNoDestroy %p n", dwater::current_thread::Tid(), this);
    }

    ~TestNoDestroy() {
        printf("tid = %d, destructing TestNoDestroy %p n", dwater::current_thread::Tid(), this);
    }
};

void ThreadFunc() {
    printf("tid = %d, %p name = %s\n",
            dwater::current_thread::Tid(),
            &dwater::Singleton<Test>::Instance(),
            dwater::Singleton<Test>::Instance().Name().c_str());
    dwater::Singleton<Test>::Instance().SetName("only one, changed");
}

int main() {
    dwater::Singleton<Test>::Instance().SetName("only one");
    dwater::Thread t1(ThreadFunc);
    t1.Start();
    t1.Join();
    printf("tid = %d, %p name = %s\n",
            dwater::current_thread::Tid(),
            &dwater::Singleton<Test>::Instance(),
            dwater::Singleton<Test>::Instance().Name().c_str());
    printf("with valgrind, you should see %zd-byte memory leak.\n", sizeof(TestNoDestroy));
}

