#include "lmock.h"
#include "minunit.h"

#include <assert.h>
#include <unistd.h>

#include <iostream>
using namespace std;

#pragma GCC diagnostic ignored "-Wunused-parameter"

//mock全局函数（任意全局函数，包括依赖的本模块、其它模块代码，以及第三方库函数，和系统函数）

//原始全局函数
int global(int a, int b)
{
    return a + b;
}

//对应的桩函数
int fake_global(int a, int b)
{
    //校验参数正确性，确定被测代码传入了正确的值
    assert(a == 3);
    assert(b == 2);
    //给一个返回值，配合被测代码走特定分支
    return a - b;
}

MU_TEST(global)
{
    cout << "global ..." << endl;

    //直接调用原函数
    assert(global(3, 2) == 5);

    //执行mock替换
    assert(0 == mock(&global, &fake_global));

    //调用mock后的函数，可以看到返回值变了
    assert(global(3, 2) == 1);

    //结束mock
    reset();

    //函数行为恢复
    assert(global(3, 2) == 5);

    return NULL;
}

class A {
public:
    int member(int a) { return ++a; }
    static int static_member(int a) { return 200; }
    virtual int virtual_member() { return 400; }
};

//mock普通成员函数(支持模板类、函数)
int fake_member(A* pTihs, int a)
{
    return --a;
}

MU_TEST(member)
{
    cout << "member ..." << endl;
    A a;
    assert(a.member(100) == 101);
    mock(&A::member, fake_member);
    assert(a.member(100) == 99);
    reset();
    assert(a.member(100) == 101);

    return NULL;
}

//mock静态成员函数
int fake_static_member()
{
    return 300;
}

MU_TEST(static_member)
{
    cout << "static member ..." << endl;
    assert(A::static_member(200) == 200);
    mock(&A::static_member, fake_static_member);
    assert(A::static_member(100) == 300);
    reset();
    assert(A::static_member(200) == 200);

    return NULL;
}

//mock虚函数
int fake_virtual_member(A* pThis)
{
    return 500;
}

MU_TEST(virtual_member)
{
    cout << "virtual member ..." << endl;
    A a;
    assert(a.virtual_member() == 400);
    //虚函数mock需要多传一个相关类的对象，任意一个对象即可，跟实际代码中的对象没有关系
    A a_obj;
    mock(&A::virtual_member, fake_virtual_member, &a_obj);
    assert(a.virtual_member() == 500);
    reset();
    assert(a.virtual_member() == 400);

    return NULL;
}

//mock系统函数
int fake_write(int, char*, int)
{
    return 100;
}

MU_TEST(system)
{
    cout << "system function ..." << endl;
    assert(write(5, "hello", 5) == -1);
    mock(write, fake_write);
    assert(write(5, "hello", 5) == 100);
    reset();
    assert(write(5, "hello", 5) == -1);

    return NULL;
}

//另外第三方库，如mysql，main, 函数也是可以支持mock的

RUN_TESTS_EX()

int fake_main(int argc, char* argv[])
{
    if (argc < 1) {
        return -1;
    }
    debug("----- RUNNING: %s", argv[0]);
    printf("----\nRUNNING: %s\n", argv[0]);
    char* result = all_tests();
    if (result != 0) {
        printf("FAILED: %s\n", result);
    } else {
        printf("ALL TESTS PASSED\n");
    }
    printf("Tests run: %d\n", tests_run);
    exit(result != 0);
}

__attribute((constructor)) void mock_main()
{
    mock(main, fake_main);
    reset();
}