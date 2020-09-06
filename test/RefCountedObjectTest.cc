#include "gtest/gtest.h"
#include "RefCountedObject.h"
#include "Allocator.h"

class MockRefCountedObject : public Common::RefCountedObject {
public:
	MockRefCountedObject() : a(1) {}
	virtual ~MockRefCountedObject() {}
	i32 a;
};

void RefCountedObjectTest_thread_ref_counter(Common::RefCounter * counter, i32 count) {
	std::this_thread::sleep_for(std::chrono::milliseconds(10));
	while (count-- > 0) {
		(*counter)++;
	}
}

void RefCountedObjectTest_thread_counter(i32 * counter, i32 count) {
	std::this_thread::sleep_for(std::chrono::milliseconds(10));
	while (count-- > 0) {
		(*counter)++;
	}
}

TEST(RefCountedObjectTest, counter) {
	Common::RefCounter counter;
	EXPECT_EQ(static_cast<i32>(counter), 1);
	EXPECT_EQ(static_cast<i32>(counter++), 1);
	EXPECT_EQ(static_cast<i32>(++counter), 3);
	EXPECT_EQ(static_cast<i32>(--counter), 2);
	EXPECT_EQ(static_cast<i32>(counter--), 2);
	EXPECT_EQ(static_cast<i32>(counter), 1);
}

TEST(RefCountedObjectTest, multi_ref_counter) {
	Common::RefCounter counter;
	std::thread threads[5];
	for (i32 i = 0; i < 5; i++) {
		threads[i] = std::thread(RefCountedObjectTest_thread_ref_counter, &counter, 10);
	}
	for (auto & t: threads) {
		t.join();
	}
	EXPECT_EQ(static_cast<i32>(counter), 51);
	std::printf("multithread RefCounter count=%d\n", static_cast<i32>(counter));
}

TEST(RefCountedObjectTest, multi_counter) {
	i32 counter = 0;
	std::thread threads[5];
	for (i32 i = 0; i < 5; i++) {
		threads[i] = std::thread(RefCountedObjectTest_thread_counter, &counter, 10);
	}
	for (auto & t: threads) {
		t.join();
	}
	EXPECT_LE(counter, 50);
	std::printf("multithread Counter count=%d\n", counter);
}

TEST(RefCountedObjectTest, ref) {
	MockRefCountedObject ref_object;
	EXPECT_EQ(ref_object.ReferenceCount(), 1);
	ref_object.Duplicate();
	EXPECT_EQ(ref_object.ReferenceCount(), 2);
	ref_object.Duplicate();
	EXPECT_EQ(ref_object.ReferenceCount(), 3);
	ref_object.Release();
	EXPECT_EQ(ref_object.ReferenceCount(), 2);
	ref_object.Release();
	EXPECT_EQ(ref_object.ReferenceCount(), 1);
}

TEST(RefCountedObjectTest, ref_ptr) {
	MockRefCountedObject * ref_object = new MockRefCountedObject();
	EXPECT_EQ(ref_object->ReferenceCount(), 1);
	ref_object->Duplicate();
	EXPECT_EQ(ref_object->ReferenceCount(), 2);
	ref_object->Release();
	EXPECT_EQ(ref_object->ReferenceCount(), 1);
	ref_object->Release();
}

TEST(RefCountedObjectTest, del) {
	MockRefCountedObject * ref_object = new MockRefCountedObject();
	EXPECT_EQ(ref_object->ReferenceCount(), 1);
	delete ref_object;
}

class MockStrongRefObject : public Common::StrongRefObject {
public:
	MockStrongRefObject(i32 v) : value_(v) {}
	virtual ~MockStrongRefObject() {}
	i32 value_;
};

TEST(RefCountedObjectTest, strong_ref) {
	MockStrongRefObject object(1);
	EXPECT_EQ(object.ReferenceCount(), 1);
	EXPECT_EQ(object.value_, 1);
}

TEST(RefCountedObjectTest, weak_ref) {
	MockStrongRefObject object(2);
	EXPECT_EQ(object.ReferenceCount(), 1);
	EXPECT_EQ(object.value_, 2);
	Common::WeakReference * ref = object.IncWeakRef();
	EXPECT_EQ(ref->ReferenceCount(), 2);
	EXPECT_TRUE(ref->Lock() == &object);
	EXPECT_EQ(object.ReferenceCount(), 2);
	ref->Release();
	EXPECT_EQ(ref->ReferenceCount(), 1);
	object.Release();
	EXPECT_EQ(object.ReferenceCount(), 1);
}

TEST(RefCountedObjectTest, strong_release) {
	MockStrongRefObject * object = new MockStrongRefObject(3);
	EXPECT_EQ(object->ReferenceCount(), 1);
	EXPECT_EQ(object->value_, 3);
	object->Release();
}

TEST(RefCountedObjectTest, strong_del) {
	MockStrongRefObject * object = new MockStrongRefObject(4);
	EXPECT_EQ(object->ReferenceCount(), 1);
	EXPECT_EQ(object->value_, 4);
	delete object;
}

TEST(RefCountedObjectTest, weak_inc) {
	MockStrongRefObject * object = new MockStrongRefObject(5);
	EXPECT_EQ(object->value_, 5);
	Common::WeakReference * ref = object->IncWeakRef();
	EXPECT_EQ(ref->ReferenceCount(), 2);
	EXPECT_TRUE(ref->Lock() == object);
	EXPECT_EQ(object->ReferenceCount(), 2);
	Common::WeakReference * ref2 = object->IncWeakRef();
	EXPECT_EQ(ref->ReferenceCount(), 3);
	EXPECT_EQ(ref2->ReferenceCount(), ref->ReferenceCount());
	EXPECT_TRUE(ref->Lock() == ref2->Lock());
	EXPECT_EQ(object->ReferenceCount(), 4);
	EXPECT_TRUE(ref2->Lock() == object);
	EXPECT_EQ(object->ReferenceCount(), 5);
	ref->Release();
	ref2->Release();
	delete object;
}

TEST(RefCountedObjectTest, weak_release_after_delete) {
	MockStrongRefObject * object = new MockStrongRefObject(6);
	EXPECT_EQ(object->value_, 6);
	Common::WeakReference * ref = object->IncWeakRef();
	EXPECT_EQ(ref->ReferenceCount(), 2);
	EXPECT_TRUE(ref->Lock() == object);
	Common::WeakReference * ref2 = object->IncWeakRef();
	EXPECT_EQ(ref->ReferenceCount(), 3);
	EXPECT_EQ(ref2->ReferenceCount(), ref->ReferenceCount());
	EXPECT_TRUE(ref->Lock() == ref2->Lock());
	EXPECT_TRUE(ref2->Lock() == object);
	ref->Release();
	delete object;
	ref2->Release();
}

TEST(RefCountedObjectTest, weak_all_release_after_delete) {
	MockStrongRefObject * object = new MockStrongRefObject(7);
	EXPECT_EQ(object->value_, 7);
	Common::WeakReference * ref = object->IncWeakRef();
	EXPECT_EQ(ref->ReferenceCount(), 2);
	EXPECT_TRUE(ref->Lock() == object);
	Common::WeakReference * ref2 = object->IncWeakRef();
	EXPECT_EQ(ref->ReferenceCount(), 3);
	EXPECT_EQ(ref2->ReferenceCount(), ref->ReferenceCount());
	EXPECT_TRUE(ref->Lock() == ref2->Lock());
	EXPECT_TRUE(ref2->Lock() == object);
	delete object;
	ref->Release();
	ref2->Release();
}

TEST(RefCountedObjectTest, weak_delete_after_release) {
	MockStrongRefObject * object = new MockStrongRefObject(8);
	EXPECT_EQ(object->value_, 8);
	Common::WeakReference * ref = object->IncWeakRef();
	EXPECT_EQ(ref->ReferenceCount(), 2);
	EXPECT_TRUE(ref->Lock() == object);
	Common::WeakReference * ref2 = object->IncWeakRef();
	EXPECT_EQ(ref->ReferenceCount(), 3);
	EXPECT_EQ(ref2->ReferenceCount(), ref->ReferenceCount());
	EXPECT_TRUE(ref->Lock() == ref2->Lock());
	EXPECT_TRUE(ref2->Lock() == object);
	ref->Release();
	ref2->Release();
	delete object;
}