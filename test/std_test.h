#ifndef __UNIT_TEST_STD_TEST_H__
#define __UNIT_TEST_STD_TEST_H__

#include "list.hpp"
#include "queue.hpp"
#include "vector.hpp"
#include <functional>

class TNode : public BaseList::BaseNode
{
public:
    explicit TNode(int data) : data_(data) {}
    int data_;
};

class TestClass
{
public:
    typedef std::function<int*(int)> TCallback;
    typedef std::function<void(int * *)> TCallback2;

    int * test(int arg) {
        return new int(arg);
    }

    void test2(int * * arg) {
        *arg = new int(888);
    }

    void SetCallback(TCallback cb) {
        cb_ = cb;
    }

    void SetCallback2(TCallback2 cb) {
        cb2_ = cb;
    }

    int * run() {
        if (cb_) {
            return cb_(100);
        } else {
            return nullptr;
        }
    }

    void run2(int * * arg) {
        if (cb2_) {
            cb2_(arg);
        } else {
            *arg = nullptr;
        }
    }

private:
    TCallback cb_;
    TCallback2 cb2_;
};

#endif