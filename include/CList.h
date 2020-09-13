/*
 * MIT License
 *
 * Copyright (c) 2019 jewmin
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef Common_CList_INCLUDED
#define Common_CList_INCLUDED

#include "Common.h"
#include "CObject.h"
#include "Allocator.h"

namespace Common {

class CListNode : public CObject {
public:
	CListNode() : next_(nullptr), prev_(nullptr) {}
	virtual ~CListNode() {}
	void Link(CListNode * node);
	void Unlink();

protected:
	CListNode * next_;
	CListNode * prev_;
};

template<typename T>
class TListNode : public CListNode {
public:
	TListNode() {}
	template<typename... Args> TListNode(Args &&... args) : data_(std::forward<Args>(args)...) {}
	virtual ~TListNode() {}

protected:
	T data_;
};

template<typename T>
class TListIterator : public CObject {
	typedef TListIterator<T>	Self;
	typedef TListNode<T>		Node;
	typedef T					Value;
	typedef T *					Pointer;
	typedef T &					Reference;

public:
	TListIterator() : node_(nullptr) {}
	explicit TListIterator(CListNode * node) : node_(node) {}

	Reference operator*() const;
	Pointer operator->() const;
	Self & operator++();
	Self operator++(i32);
	Self & operator--();
	Self operator--(i32);
	bool operator==(const Self & other) const;
	bool operator!=(const Self & other) const;

private:
	CListNode * node_;
};

template<typename T>
class TListConstIterator : public CObject {
	typedef TListConstIterator<T>	Self;
	typedef const TListNode<T>		Node;
	typedef TListIterator<T>		Iterator;
	typedef T						Value;
	typedef const T *				Pointer;
	typedef const T &				Reference;

public:
	TListConstIterator() : node_(nullptr) {}
	explicit TListConstIterator(const CListNode * node) : node_(node) {}
	TListConstIterator(const Iterator & iter) : node_(iter.node_) {}

	Reference operator*() const;
	Pointer operator->() const;
	Self & operator++();
	Self operator++(i32);
	Self & operator--();
	Self operator--(i32);
	bool operator==(const Self & other) const;
	bool operator!=(const Self & other) const;

private:
	const CListNode * node_;
};

template<typename T>
class CList : public CObject {
	typedef TListNode<T> Node;

public:
	CList() { Init(); }
	~CList() { Clear(); }

protected:
	void Init();
	void Clear();
	Node * Alloc();
	void Free(Node * node);

protected:
	CListNode node_;
};

template<typename T>
class TList : public CList {
	typedef T						Value;
	typedef TListNode<T>			Node;
	typedef T *						Pointer;
	typedef const T *				ConstPointer;
	typedef T &						Reference;
	typedef const T &				ConstReference;
	typedef TListIterator<T>		Iterator;
	typedef TListConstIterator<T>	ConstIterator;

public:

protected:
	Node * CreateNode(const Value & value);
	template<typename... Args>
	Node * CreateNode(Args &&... args);

private:

};

//*********************************************************************
//CListNode
//*********************************************************************

inline void CListNode::Link(CListNode * node) {
	next_ = node;
	prev_ = node->prev_;
	node->prev_->next_ = this;
	node->prev_ = this;
}

inline void CListNode::Unlink() {
	CListNode * next_node = next_;
	CListNode * prev_node = prev_;
	prev_node->next_ = next_node;
	next_node->prev_ = prev_node;
}

//*********************************************************************
//TListIterator
//*********************************************************************

template<typename T>
inline TListIterator<T>::Reference TListIterator<T>::operator*() const {
	return static_cast<Node *>(node_)->data_;
}

template<typename T>
inline TListIterator<T>::Pointer TListIterator<T>::operator->() const {
	return std::addressof(static_cast<Node *>(node_)->data_);
}

template<typename T>
inline TListIterator<T>::Self & TListIterator<T>::operator++() {
	node_ = node_->next_;
	return *this;
}

template<typename T>
inline TListIterator<T>::Self TListIterator<T>::operator++(i32) {
	Self tmp = *this;
	node_ = node_->next_;
	return tmp;
}

template<typename T>
inline TListIterator<T>::Self & TListIterator<T>::operator--() {
	node_ = node_->prev_;
	return *this;
}

template<typename T>
inline TListIterator<T>::Self TListIterator<T>::operator--(i32) {
	Self tmp = *this;
	node_ = node_->prev_;
	return tmp;
}

template<typename T>
inline bool TListIterator<T>::operator==(const Self & other) const {
	return node_ == other.node_;
}

template<typename T>
inline bool TListIterator<T>::operator!=(const Self & other) const {
	return node_ != other.node_;
}

//*********************************************************************
//TListConstIterator
//*********************************************************************

template<typename T>
inline TListConstIterator<T>::Reference TListConstIterator<T>::operator*() const {
	return static_cast<Node *>(node_)->data_;
}

template<typename T>
inline TListConstIterator<T>::Pointer TListConstIterator<T>::operator->() const {
	return std::addressof(static_cast<Node *>(node_)->data_);
}

template<typename T>
inline TListConstIterator<T>::Self & TListConstIterator<T>::operator++() {
	node_ = node_->next_;
	return *this;
}

template<typename T>
inline TListConstIterator<T>::Self TListConstIterator<T>::operator++(i32) {
	Self tmp = *this;
	node_ = node_->next_;
	return tmp;
}

template<typename T>
inline TListConstIterator<T>::Self & TListConstIterator<T>::operator--() {
	node_ = node_->prev_;
	return *this;
}

template<typename T>
inline TListConstIterator<T>::Self TListConstIterator<T>::operator--(i32) {
	Self tmp = *this;
	node_ = node_->prev_;
	return tmp;
}

template<typename T>
inline bool TListConstIterator<T>::operator==(const Self & other) const {
	return node_ == other.node_;
}

template<typename T>
inline bool TListConstIterator<T>::operator!=(const Self & other) const {
	return node_ != other.node_;
}

//*********************************************************************
//CList
//*********************************************************************

template<typename T>
inline void CList<T>::Init() {
	node_.next_ = &node_;
	node_.prev_ = &node_;
}

template<typename T>
inline void CList<T>::Clear() {
	Node * cur = static_cast<Node *>(node_.next_);
	while (cur != &node_) {
		Node * tmp = cur;
		cur = static_cast<Node *>(cur->next_);
		tmp->~TListNode();
		Free(tmp);
	}
}

template<typename T>
inline CList<T>::Node * CList<T>::Alloc() {
	return static_cast<Node *>(jc_malloc(sizeof(Node)));
}

template<typename T>
inline void CList<T>::Free(Node * node) {
	jc_free(node);
}

}

#endif