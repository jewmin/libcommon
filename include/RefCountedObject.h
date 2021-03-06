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

#ifndef Common_RefCountedObject_INCLUDED
#define Common_RefCountedObject_INCLUDED

#include "Common.h"
#include "CObject.h"

namespace Common {

class COMMON_EXTERN RefCounter : public CObject {
public:
	RefCounter();
	~RefCounter();

	i32 operator++();
	i32 operator++(i32);
	i32 operator--();
	i32 operator--(i32);
	operator int();

private:
	RefCounter(RefCounter &&) = delete;
	RefCounter(const RefCounter &) = delete;
	RefCounter & operator=(RefCounter &&) = delete;
	RefCounter & operator=(const RefCounter &) = delete;

private:
	mutable std::atomic<i32> counter_;
};

class COMMON_EXTERN RefCountedObject : public CObject {
public:
	virtual ~RefCountedObject();

	void Duplicate() const;
	void Release();
	i32 ReferenceCount() const;

protected:
	RefCountedObject();

private:
	RefCountedObject(RefCountedObject &&) = delete;
	RefCountedObject(const RefCountedObject &) = delete;
	RefCountedObject & operator=(RefCountedObject &&) = delete;
	RefCountedObject & operator=(const RefCountedObject &) = delete;

private:
	mutable RefCounter counter_;
};

class StrongRefObject;
class COMMON_EXTERN WeakReference : public RefCountedObject {
	friend class StrongRefObject;
public:
	virtual ~WeakReference();
	
	StrongRefObject * Lock() const;

protected:
	WeakReference();

private:
	WeakReference(WeakReference &&) = delete;
	WeakReference(const WeakReference &) = delete;
	WeakReference & operator=(WeakReference &&) = delete;
	WeakReference & operator=(const WeakReference &) = delete;

private:
	StrongRefObject * object_;
};

class COMMON_EXTERN StrongRefObject : public RefCountedObject {
public:
	StrongRefObject();
	virtual ~StrongRefObject();

	WeakReference * IncWeakRef();

protected:
	void ClearWeakReferences();

private:
	StrongRefObject(StrongRefObject &&) = delete;
	StrongRefObject(const StrongRefObject &) = delete;
	StrongRefObject & operator=(StrongRefObject &&) = delete;
	StrongRefObject & operator=(const StrongRefObject &) = delete;

private:
	WeakReference * weak_reference_;
};

//*********************************************************************
//RefCounter
//*********************************************************************

inline i32 RefCounter::operator++() {
	return counter_.fetch_add(1, std::memory_order_relaxed) + 1;
}

inline i32 RefCounter::operator++(i32) {
	return counter_.fetch_add(1, std::memory_order_relaxed);
}

inline i32 RefCounter::operator--() {
	return counter_.fetch_sub(1, std::memory_order_acquire) - 1;
}

inline i32 RefCounter::operator--(i32) {
	return counter_.fetch_sub(1, std::memory_order_acquire);
}

inline RefCounter::operator int() {
	return counter_;
}

//*********************************************************************
//RefCountedObject
//*********************************************************************

inline void RefCountedObject::Duplicate() const {
	counter_++;
}

inline void RefCountedObject::Release() {
	assert(counter_ > 0);
	if (--counter_ == 0) {
		delete this;
	}
}

inline i32 RefCountedObject::ReferenceCount() const {
	return counter_;
}

//*********************************************************************
//WeakReference
//*********************************************************************

inline StrongRefObject * WeakReference::Lock() const {
	if (object_) {
		object_->Duplicate();
	}
	return object_;
}

}

#endif