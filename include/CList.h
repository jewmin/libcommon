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
	CListNode(CListNode * next, CListNode * prev): next_(next), prev_(prev) {}
	virtual ~CListNode() {}
	// 在指定node前插入
	void Link(CListNode * node);
	void Unlink();

private:
	CListNode(CListNode &&) = delete;
	CListNode(const CListNode &) = delete;
	CListNode & operator=(CListNode &&) = delete;
	CListNode & operator=(const CListNode &) = delete;

public:
	CListNode * next_;
	CListNode * prev_;
};

class CListNodeHeader : public CListNode {
public:
	CListNodeHeader() : CListNode(this, this), count_(0) {}
	CListNodeHeader(CListNodeHeader && other) : CListNode(other.next_, other.prev_), count_(other.count_) {
		if (other.next_ == std::addressof(other)) {
			next_ = prev_ = this;
		} else {
			next_->prev_ = prev_->next_ = this;
			other.Init();
		}
	}
	virtual ~CListNodeHeader() {}

	void Init();

private:
	CListNodeHeader(const CListNodeHeader &) = delete;
	CListNodeHeader & operator=(CListNodeHeader &&) = delete;
	CListNodeHeader & operator=(const CListNodeHeader &) = delete;

public:
	i32 count_;
};

template<typename T>
class TListNode : public CListNode {
public:
	TListNode() {}
	template<typename... Args> TListNode(Args &&... args) : data_(std::forward<Args>(args)...) {}
	virtual ~TListNode() {}

private:
	TListNode(TListNode &&) = delete;
	TListNode(const TListNode &) = delete;
	TListNode & operator=(TListNode &&) = delete;
	TListNode & operator=(const TListNode &) = delete;

public:
	T data_;
};

template<typename T>
class TList;

template<typename T>
class TListConstIterator;

template<typename T>
class TListIterator : public CObject {
	friend class TList<T>;
	friend class TListConstIterator<T>;
	typedef TListIterator<T>	Self;
	typedef TListNode<T>		Node;
	typedef T					Value;
	typedef T *					Pointer;
	typedef T &					Reference;

public:
	TListIterator() : node_(nullptr) {}
	explicit TListIterator(CListNode * node) : node_(node) {}
	TListIterator(const TListIterator & other) : node_(other.node_) {}
	TListIterator & operator=(const TListIterator & other) {
		if (this != std::addressof(other)) {
			node_ = other.node_;
		}
		return *this;
	}

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
	friend class TList<T>;
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
	TListConstIterator(const TListConstIterator & other) : node_(other.node_) {}
	TListConstIterator & operator=(const TListConstIterator & other) {
		if (this != std::addressof(other)) {
			node_ = other.node_;
		}
		return *this;
	}

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
class TList : public CObject {
public:
	typedef T						Value;
	typedef TListNode<T>			Node;
	typedef T *						Pointer;
	typedef const T *				ConstPointer;
	typedef T &						Reference;
	typedef const T &				ConstReference;
	typedef TListIterator<T>		Iterator;
	typedef TListConstIterator<T>	ConstIterator;

public:
	TList() {}
	TList(TList && other) : head_(std::move(other.head_)) {}
	~TList() { FreeAllNode(); }

	i32 Size() const;
	bool Empty() const;
	void Clear();

	Iterator Begin();
	ConstIterator Begin() const;
	Iterator End();
	ConstIterator End() const;
	Reference Front();
	ConstReference Front() const;
	Reference Back();
	ConstReference Back() const;

	void PushFront(const Value & value);
	void PushFront(Value && value);
	template<typename... Args> void EmplaceFront(Args &&... args);
	void PopFront();

	void PushBack(const Value & value);
	void PushBack(Value && value);
	template<typename... Args> void EmplaceBack(Args &&... args);
	void PopBack();

	Iterator Erase(Iterator it);

protected:
	// void SetSize(i32 count);
	void IncSize(i32 count);
	void DecSize(i32 count);
	void FreeAllNode();
	Node * AllocNode();
	void FreeNode(Node * node);
	Node * CreateNode(const Value & value);
	template<typename... Args> Node * CreateNode(Args &&... args);
	void DestroyNode(Node * node);
	void InsertNode(Iterator it, const Value & value);
	template<typename... Args> void InsertNode(Iterator it, Args &&... args);
	void EraseNode(Iterator it);

private:
	TList(const TList &) = delete;
	TList & operator=(TList &&) = delete;
	TList & operator=(const TList &) = delete;

private:
	CListNodeHeader head_;
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
//CListNodeHeader
//*********************************************************************

inline void CListNodeHeader::Init() {
	next_ = prev_ = this;
	count_ = 0;
}

//*********************************************************************
//TListIterator
//*********************************************************************

template<typename T>
inline typename TListIterator<T>::Reference TListIterator<T>::operator*() const {
	return static_cast<Node *>(node_)->data_;
}

template<typename T>
inline typename TListIterator<T>::Pointer TListIterator<T>::operator->() const {
	return std::addressof(static_cast<Node *>(node_)->data_);
}

template<typename T>
inline typename TListIterator<T>::Self & TListIterator<T>::operator++() {
	node_ = node_->next_;
	return *this;
}

template<typename T>
inline typename TListIterator<T>::Self TListIterator<T>::operator++(i32) {
	Self tmp = *this;
	node_ = node_->next_;
	return tmp;
}

template<typename T>
inline typename TListIterator<T>::Self & TListIterator<T>::operator--() {
	node_ = node_->prev_;
	return *this;
}

template<typename T>
inline typename TListIterator<T>::Self TListIterator<T>::operator--(i32) {
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
inline typename TListConstIterator<T>::Reference TListConstIterator<T>::operator*() const {
	return static_cast<Node *>(node_)->data_;
}

template<typename T>
inline typename TListConstIterator<T>::Pointer TListConstIterator<T>::operator->() const {
	return std::addressof(static_cast<Node *>(node_)->data_);
}

template<typename T>
inline typename TListConstIterator<T>::Self & TListConstIterator<T>::operator++() {
	node_ = node_->next_;
	return *this;
}

template<typename T>
inline typename TListConstIterator<T>::Self TListConstIterator<T>::operator++(i32) {
	Self tmp = *this;
	node_ = node_->next_;
	return tmp;
}

template<typename T>
inline typename TListConstIterator<T>::Self & TListConstIterator<T>::operator--() {
	node_ = node_->prev_;
	return *this;
}

template<typename T>
inline typename TListConstIterator<T>::Self TListConstIterator<T>::operator--(i32) {
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
//TList
//*********************************************************************

template<typename T>
inline i32 TList<T>::Size() const {
	return head_.count_;
}

// template<typename T>
// inline void TList<T>::SetSize(i32 count) {
// 	head_.count_ = count;
// }

template<typename T>
inline void TList<T>::IncSize(i32 count) {
	head_.count_ += count;
}

template<typename T>
inline void TList<T>::DecSize(i32 count) {
	head_.count_ -= count;
}

template<typename T>
inline bool TList<T>::Empty() const {
	return head_.next_ == &head_;
}

template<typename T>
inline void TList<T>::Clear() {
	FreeAllNode();
	head_.Init();
}

template<typename T>
inline typename TList<T>::Iterator TList<T>::Begin() {
	return Iterator(head_.next_);
}

template<typename T>
inline typename TList<T>::ConstIterator TList<T>::Begin() const {
	return ConstIterator(head_.next_);
}

template<typename T>
inline typename TList<T>::Iterator TList<T>::End() {
	return Iterator(&head_);
}

template<typename T>
inline typename TList<T>::ConstIterator TList<T>::End() const {
	return ConstIterator(&head_);
}

template<typename T>
inline typename TList<T>::Reference TList<T>::Front() {
	return *Begin();
}

template<typename T>
inline typename TList<T>::ConstReference TList<T>::Front() const {
	return *Begin();
}

template<typename T>
inline typename TList<T>::Reference TList<T>::Back() {
	Iterator tmp(End());
	--tmp;
	return *tmp;
}

template<typename T>
inline typename TList<T>::ConstReference TList<T>::Back() const {
	ConstIterator tmp(End());
	--tmp;
	return *tmp;
}

template<typename T>
inline void TList<T>::PushFront(const Value & value) {
	InsertNode(Begin(), value);
}

template<typename T>
inline void TList<T>::PushFront(Value && value) {
	InsertNode(Begin(), std::move(value));
}

template<typename T>
template<typename... Args>
inline void TList<T>::EmplaceFront(Args &&... args) {
	InsertNode(Begin(), std::forward<Args>(args)...);
}

template<typename T>
inline void TList<T>::PopFront() {
	EraseNode(Begin());
}

template<typename T>
inline void TList<T>::PushBack(const Value & value) {
	InsertNode(End(), value);
}

template<typename T>
inline void TList<T>::PushBack(Value && value) {
	InsertNode(End(), std::move(value));
}

template<typename T>
template<typename... Args>
inline void TList<T>::EmplaceBack(Args &&... args) {
	InsertNode(End(), std::forward<Args>(args)...);
}

template<typename T>
inline void TList<T>::PopBack() {
	EraseNode(Iterator(head_.prev_));
}

template<typename T>
inline typename TList<T>::Iterator TList<T>::Erase(Iterator it) {
	Iterator next(it.node_->next_);
	EraseNode(it);
	return next;
}

template<typename T>
inline void TList<T>::FreeAllNode() {
	CListNode * cur = head_.next_;
	while (cur != &head_) {
		Node * tmp = static_cast<Node *>(cur);
		cur = cur->next_;
		DestroyNode(tmp);
		FreeNode(tmp);
	}
}

template<typename T>
inline typename TList<T>::Node * TList<T>::AllocNode() {
	return static_cast<Node *>(jc_malloc(sizeof(Node)));
}

template<typename T>
inline void TList<T>::FreeNode(Node * node) {
	jc_free(node);
}

template<typename T>
inline typename TList<T>::Node * TList<T>::CreateNode(const Value & value) {
	Node * node = AllocNode();
	new(node)Node(value);
	return node;
}

template<typename T>
template<typename... Args>
inline typename TList<T>::Node * TList<T>::CreateNode(Args &&... args) {
	Node * node = AllocNode();
	new(node)Node(std::forward<Args>(args)...);
	return node;
}

template<typename T>
inline void TList<T>::DestroyNode(Node * node) {
	node->~TListNode();
}

template<typename T>
inline void TList<T>::InsertNode(Iterator it, const Value & value) {
	Node * node = CreateNode(value);
	node->Link(it.node_);
	IncSize(1);
}

template<typename T>
template<typename... Args>
inline void TList<T>::InsertNode(Iterator it, Args &&... args) {
	Node * node = CreateNode(std::forward<Args>(args)...);
	node->Link(it.node_);
	IncSize(1);
}

template<typename T>
inline void TList<T>::EraseNode(Iterator it) {
	DecSize(1);
	it.node_->Unlink();
	Node * tmp = static_cast<Node *>(it.node_);
	DestroyNode(tmp);
	FreeNode(tmp);
}

}

#endif