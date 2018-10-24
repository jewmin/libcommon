#ifndef __LIBCOMMON_LIST_HPP__
#define __LIBCOMMON_LIST_HPP__

#include "non_copy_able.hpp"

class BaseList : public NonCopyAble {
public:
    class BaseNode {
    public:
        BaseNode * Next() const {
            return next_;
        }
        BaseNode * Prev() const {
            return prev_;
        }
        void Next(BaseNode * next) {
            next_ = next;
            if (next) {
                next->prev_ = this;
            }
        }
        void Prev(BaseNode * prev) {
            prev_ = prev;
            if (prev) {
                prev->next_ = this;
            }
        }
        void AddToList(BaseList * list) {
            list_ = list;
        }
        void RemoveFromList() {
            if (list_) {
                list_->RemoveNode(this);
            }
        }

    protected:
        BaseNode() : next_(nullptr), prev_(nullptr), list_(nullptr) {

        }
        ~BaseNode() {
            RemoveFromList();
            next_ = prev_ = nullptr;
            list_ = nullptr;
        }

    private:
        friend class BaseList;

        void Unlink() {
            if (prev_) {
                prev_->next_ = next_;
            }
            if (next_) {
                next_->prev_ = prev_;
            }

            next_ = prev_ = nullptr;
            list_ = nullptr;
        }

    private:
        BaseNode * next_;
        BaseNode * prev_;
        BaseList * list_;
    };

    BaseList() : head_(nullptr), tail_(nullptr), count_(0) {

    }

    void PushNode(BaseNode * node) {
        node->AddToList(this);

        node->Next(head_);
        head_ = node;
        if (!tail_) {
            tail_ = node;
        }

        ++count_;
    }

    void PushBackNode(BaseNode * node) {
        node->AddToList(this);

        node->Prev(tail_);
        tail_ = node;
        if (!head_) {
            head_ = node;
        }

        ++count_;
    }

    BaseNode * PopNode() {
        BaseNode * node = head_;

        if (node) {
            RemoveNode(node);
        }

        return node;
    }

    BaseNode * PopBackNode() {
        BaseNode * node = tail_;

        if (node) {
            RemoveNode(node);
        }

        return node;
    }

    BaseNode * Head() const {
        return head_;
    }
    BaseNode * Tail() const {
        return tail_;
    }
    int Count() const {
        return count_;
    }
    bool IsEmpty() const {
        return 0 == count_;
    }

private:
    void RemoveNode(BaseNode * node) {
        if (node == head_) {
            head_ = node->Next();
        }
        if (node == tail_) {
            tail_ = node->Prev();
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
class TNodeList : public BaseList
{
public:
    void PushNode(T * node) {
        BaseList::PushNode(node);
    }
    void PushBackNode(T * node) {
        BaseList::PushBackNode(node);
    }
    T * PopNode() {
        return static_cast<T *>(BaseList::PopNode());
    }
    T * PopBackNode() {
        return static_cast<T *>(BaseList::PopBackNode());
    }
    T * Head() const {
        return static_cast<T *>(BaseList::Head());
    }
    T * Tail() const {
        return static_cast<T *>(BaseList::Tail());
    }

    static T * Next(const T * node) {
        return static_cast<T *>(node->Next());
    }
    static T * Prev(const T * node) {
        return static_cast<T *>(node->Prev());
    }
};

#endif