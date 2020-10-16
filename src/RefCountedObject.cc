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

#include "RefCountedObject.h"

namespace Common {

RefCounter::RefCounter() : counter_(1) {
}

RefCounter::~RefCounter() {
}

RefCountedObject::RefCountedObject() {
}

RefCountedObject::~RefCountedObject() {
}

WeakReference::WeakReference() : object_(nullptr) {
}

WeakReference::~WeakReference() {
}

StrongRefObject::StrongRefObject() : weak_reference_(nullptr) {
}

StrongRefObject::~StrongRefObject() {
	ClearWeakReferences();
}

WeakReference * StrongRefObject::IncWeakRef() {
	if (!weak_reference_) {
		weak_reference_ = new WeakReference();
		weak_reference_->object_ = this;
	}
	weak_reference_->Duplicate();
	return weak_reference_;
}

void StrongRefObject::ClearWeakReferences() {
	if (weak_reference_) {
		weak_reference_->object_ = nullptr;
		weak_reference_->Release();
		weak_reference_ = nullptr;
	}
}

}