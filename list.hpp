#ifndef __LIBCOMMON_LIST_HPP__
#define __LIBCOMMON_LIST_HPP__

#include "non_copy_able.hpp"

class BaseList : public NonCopyAble {
public:
    class BaseNode {
    public:
        inline BaseNode * Right() const {
            return right_;
        }

        inline BaseNode * Left() const {
            return left_;
        }

        inline void Right(BaseNode * node) {
            right_ = node;
            if (node) {
                node->left_ = this;
            }
        }

        inline void Left(BaseNode * node) {
            left_ = node;
            if (node) {
                node->right_ = this;
            }
        }

        inline void AddToList(BaseList * list) {
            list_ = list;
        }

        inline void RemoveFromList() {
            if (list_) {
                list_->RemoveNode(this);
            }
        }

    protected:
        BaseNode() : right_(nullptr), left_(nullptr), list_(nullptr) {

        }

        virtual ~BaseNode() {
            RemoveFromList();
            right_ = left_ = nullptr;
            list_ = nullptr;
        }

    private:
        friend class BaseList;

        inline void Unlink() {
            if (left_) {
                left_->right_ = right_;
            }

            if (right_) {
                right_->left_ = left_;
            }

            right_ = left_ = nullptr;
            list_ = nullptr;
        }

    private:
        BaseNode * right_;
        BaseNode * left_;
        BaseList * list_;
    };

    BaseList() : head_(nullptr), tail_(nullptr), count_(0) {

    }

    inline void PushLeft(BaseNode * node) {
        node->AddToList(this);

        node->Right(head_);
        head_ = node;
        if (!tail_) {
            tail_ = node;
        }

        ++count_;
    }

    inline void PushRight(BaseNode * node) {
        node->AddToList(this);

        node->Left(tail_);
        tail_ = node;
        if (!head_) {
            head_ = node;
        }

        ++count_;
    }

    inline BaseNode * PopLeft() {
        BaseNode * node = head_;

        if (node) {
            RemoveNode(node);
        }
        
        return node;
    }

    inline BaseNode * PopRight() {
        BaseNode * node = tail_;

        if (node) {
            RemoveNode(node);
        }

        return node;
    }

    inline BaseNode * Left() const {
        return head_;
    }

    inline BaseNode * Right() const {
        return tail_;
    }

    inline int Count() const {
        return count_;
    }

    inline bool IsEmpty() const {
        return 0 == count_;
    }

private:
    inline void RemoveNode(BaseNode * node) {
        if (node == head_) {
            head_ = node->Right();
        }

        if (node == tail_) {
            tail_ = node->Left();
        }

        node->Unlink();

        --count_;
    }

private:
    BaseNode * head_;
    BaseNode * tail_;
    int count_;
};

template<class T>
class TNodeList : public BaseList {
public:
    inline void PushLeft(T * node) {
        BaseList::PushLeft(node);
    }

    inline void PushRight(T * node) {
        BaseList::PushRight(node);
    }

    inline T * PopLeft() {
        return static_cast<T *>(BaseList::PopLeft());
    }

    inline T * PopRight() {
        return static_cast<T *>(BaseList::PopRight());
    }

    inline T * Left() const {
        return static_cast<T *>(BaseList::Left());
    }

    inline T * Right() const {
        return static_cast<T *>(BaseList::Right());
    }

    static T * Left(const T * node) {
        return static_cast<T *>(node->Left());
    }
    static T * Right(const T * node) {
        return static_cast<T *>(node->Right());
    }
};

#endif