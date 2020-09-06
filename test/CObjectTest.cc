#include "gtest/gtest.h"
#include "CObject.h"
#include "Allocator.h"

class MockObject : public Common::CObject {
public:
	MockObject() : a(1), b(1.5), c(2), d(2.5) {}
	virtual ~MockObject() {}
	i32 a;
	float b;
	i64 c;
	double d;
};

TEST(CObjectTest, alloc) {
	MockObject object;
	EXPECT_EQ(object.a, 1);
	EXPECT_FLOAT_EQ(object.b, 1.5);
	EXPECT_EQ(object.c, 2);
	EXPECT_DOUBLE_EQ(object.d, 2.5);

	MockObject * object1 = new MockObject();
	EXPECT_TRUE(object1 != nullptr);
	EXPECT_EQ(object1->a, 1);
	EXPECT_FLOAT_EQ(object1->b, 1.5);
	EXPECT_EQ(object1->c, 2);
	EXPECT_DOUBLE_EQ(object1->d, 2.5);
	delete object1;

	i8 buf[sizeof(MockObject)];
	MockObject * object2 = reinterpret_cast<MockObject *>(const_cast<i8 *>(buf));
	new(object2)MockObject();
	EXPECT_EQ(object2->a, 1);
	EXPECT_FLOAT_EQ(object2->b, 1.5);
	EXPECT_EQ(object2->c, 2);
	EXPECT_DOUBLE_EQ(object2->d, 2.5);
	object2->~MockObject();

	MockObject * objects = new MockObject[10];
	for (i32 i = 0; i < 10; i++) {
		EXPECT_TRUE(objects + i != nullptr);
		EXPECT_EQ((objects + i)->a, 1);
		EXPECT_FLOAT_EQ((objects + i)->b, 1.5);
		EXPECT_EQ((objects + i)->c, 2);
		EXPECT_DOUBLE_EQ((objects + i)->d, 2.5);
	}
	delete [] objects;
}