#ifndef __UNIT_TEST_STD_TEST_H__
#define __UNIT_TEST_STD_TEST_H__

#include "list.hpp"
#include "queue.hpp"
#include "vector.hpp"
#include "container.h"

class TNode : public BaseList::BaseNode
{
public:
    TNode(int data) : data_(data) {}
    int data_;
};

#endif