#include "gtest/gtest.h"
#include "RefCountedObject.h"
#include "Allocator.h"

class MockRefCountedObject : public Common::RefCountedObject {
public:
	MockRefCountedObject() : a(1) {}
	virtual ~MockRefCountedObject() {}
	int a;
};

TEST(RefCountedObjectTestSuite, counter) {
	Common::RefCounter counter;
	EXPECT_EQ((int)counter, 1);
	EXPECT_EQ((int)counter++, 1);
	EXPECT_EQ((int)++counter, 3);
	EXPECT_EQ((int)--counter, 2);
	EXPECT_EQ((int)counter--, 2);
	EXPECT_EQ((int)counter, 1);
}

TEST(RefCountedObjectTestSuite, ref) {
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

TEST(RefCountedObjectTestSuite, ref_ptr) {
	MockRefCountedObject * ref_object = new MockRefCountedObject();
	EXPECT_EQ(ref_object->ReferenceCount(), 1);
	ref_object->Duplicate();
	EXPECT_EQ(ref_object->ReferenceCount(), 2);
	ref_object->Release();
	EXPECT_EQ(ref_object->ReferenceCount(), 1);
	ref_object->Release();
}

TEST(RefCountedObjectTestSuite, del) {
	char buf[sizeof(MockRefCountedObject)];
	MockRefCountedObject * ref_object = reinterpret_cast<MockRefCountedObject *>(const_cast<char *>(buf));
	new(ref_object)MockRefCountedObject();
	EXPECT_EQ(ref_object->ReferenceCount(), 1);
	ref_object->Duplicate();
	EXPECT_EQ(ref_object->ReferenceCount(), 2);
	ref_object->Release();
	EXPECT_EQ(ref_object->ReferenceCount(), 1);
}

class MockStrongObject : public Common::StrongRefObject {
public:
	MockStrongObject(i32 v) : value_(v) {}
	virtual ~MockStrongObject() {}
	i32 value_;
};

TEST(RefCountedObjectTestSuite, weak) {
	MockStrongObject object(1);
	EXPECT_EQ(object.value_, 1);
}

TEST(RefCountedObjectTestSuite, weak2) {
	MockStrongObject object(2);
	EXPECT_EQ(object.value_, 2);
	Common::WeakReference * ref = object.WeakRef();
	EXPECT_EQ(ref->ReferenceCount(), 2);
	EXPECT_TRUE(ref->Get() == &object);
	ref->Release();
	EXPECT_EQ(ref->ReferenceCount(), 1);
}

TEST(RefCountedObjectTestSuite, weak3) {
	MockStrongObject * object = new MockStrongObject(3);
	EXPECT_EQ(object->value_, 3);
	delete object;
}

TEST(RefCountedObjectTestSuite, weak4) {
	MockStrongObject * object = new MockStrongObject(4);
	EXPECT_EQ(object->value_, 4);
	Common::WeakReference * ref = object->WeakRef();
	EXPECT_EQ(ref->ReferenceCount(), 2);
	EXPECT_TRUE(ref->Get() == object);
	Common::WeakReference * ref2 = object->WeakRef();
	EXPECT_EQ(ref->ReferenceCount(), 3);
	EXPECT_EQ(ref2->ReferenceCount(), ref->ReferenceCount());
	EXPECT_TRUE(ref->Get() == ref2->Get());
	EXPECT_TRUE(ref2->Get() == object);
	ref->Release();
	ref2->Release();
	delete object;
}

TEST(RefCountedObjectTestSuite, weak5) {
	MockStrongObject * object = new MockStrongObject(5);
	EXPECT_EQ(object->value_, 5);
	Common::WeakReference * ref = object->WeakRef();
	EXPECT_EQ(ref->ReferenceCount(), 2);
	EXPECT_TRUE(ref->Get() == object);
	Common::WeakReference * ref2 = object->WeakRef();
	EXPECT_EQ(ref->ReferenceCount(), 3);
	EXPECT_EQ(ref2->ReferenceCount(), ref->ReferenceCount());
	EXPECT_TRUE(ref->Get() == ref2->Get());
	EXPECT_TRUE(ref2->Get() == object);
	ref->Release();
	delete object;
	ref2->Release();
}

TEST(RefCountedObjectTestSuite, weak6) {
	MockStrongObject * object = new MockStrongObject(6);
	EXPECT_EQ(object->value_, 6);
	Common::WeakReference * ref = object->WeakRef();
	EXPECT_EQ(ref->ReferenceCount(), 2);
	EXPECT_TRUE(ref->Get() == object);
	Common::WeakReference * ref2 = object->WeakRef();
	EXPECT_EQ(ref->ReferenceCount(), 3);
	EXPECT_EQ(ref2->ReferenceCount(), ref->ReferenceCount());
	EXPECT_TRUE(ref->Get() == ref2->Get());
	EXPECT_TRUE(ref2->Get() == object);
	delete object;
	ref->Release();
	ref2->Release();
}

TEST(RefCountedObjectTestSuite, weak7) {
	MockStrongObject * object = new MockStrongObject(7);
	EXPECT_EQ(object->value_, 7);
	Common::WeakReference * ref = object->WeakRef();
	EXPECT_EQ(ref->ReferenceCount(), 2);
	EXPECT_TRUE(ref->Get() == object);
	Common::WeakReference * ref2 = object->WeakRef();
	EXPECT_EQ(ref->ReferenceCount(), 3);
	EXPECT_EQ(ref2->ReferenceCount(), ref->ReferenceCount());
	EXPECT_TRUE(ref->Get() == ref2->Get());
	EXPECT_TRUE(ref2->Get() == object);
	ref->Release();
	ref2->Release();
	delete object;
}