#include "gtest/gtest.h"
#include "CList.h"

class CListTest_MockObject {
public:
	CListTest_MockObject() : a_("") {}
	CListTest_MockObject(const i8 * str) : a_(str) {}
	CListTest_MockObject(const CListTest_MockObject & other) : a_(other.a_) {}
	CListTest_MockObject(CListTest_MockObject && other) : a_(std::move(other.a_)) {}
	~CListTest_MockObject() {}
	bool operator==(const CListTest_MockObject & other) const {
		return a_ == other.a_;
	}
	bool operator!=(const CListTest_MockObject & other) const {
		return a_ != other.a_;
	}
	std::string a_;
};

TEST(CListNodeTest, ctor) {
	Common::CListNode * n1 = new Common::CListNode();
	Common::CListNode * n2 = new Common::CListNode(n1, nullptr);
	Common::CListNode * n3 = new Common::CListNode(n2, n1);
	Common::CListNode * n4 = new Common::CListNode(nullptr, n3);

	EXPECT_TRUE(n1->next_ == nullptr);
	EXPECT_TRUE(n1->prev_ == nullptr);

	EXPECT_TRUE(n2->next_ == n1);
	EXPECT_TRUE(n2->prev_ == nullptr);
	EXPECT_TRUE(n2->next_->next_ == nullptr);
	EXPECT_TRUE(n2->next_->prev_ == nullptr);

	EXPECT_TRUE(n3->next_ == n2);
	EXPECT_TRUE(n3->prev_ == n1);
	EXPECT_TRUE(n3->next_->next_ == n1);
	EXPECT_TRUE(n3->next_->prev_ == nullptr);
	EXPECT_TRUE(n3->next_->next_->next_ == nullptr);
	EXPECT_TRUE(n3->next_->next_->prev_ == nullptr);

	EXPECT_TRUE(n4->next_ == nullptr);
	EXPECT_TRUE(n4->prev_ == n3);
	EXPECT_TRUE(n4->prev_->next_ == n2);
	EXPECT_TRUE(n4->prev_->prev_ == n1);
	EXPECT_TRUE(n4->prev_->next_->next_ == n1);
	EXPECT_TRUE(n4->prev_->next_->prev_ == nullptr);
	EXPECT_TRUE(n4->prev_->next_->next_->next_ == nullptr);
	EXPECT_TRUE(n4->prev_->next_->next_->prev_ == nullptr);

	delete n1;
	delete n2;
	delete n3;
	delete n4;
}

TEST(CListNodeTest, link) {
	Common::CListNode * n1 = new Common::CListNode();
	n1->next_ = n1->prev_ = n1;
	EXPECT_TRUE(n1->next_ == n1);
	EXPECT_TRUE(n1->prev_ == n1);

	Common::CListNode * n2 = new Common::CListNode();
	n2->Link(n1);
	EXPECT_TRUE(n2->next_ == n1);
	EXPECT_TRUE(n2->prev_ == n1);
	EXPECT_TRUE(n1->next_ == n2);
	EXPECT_TRUE(n1->prev_ == n2);

	Common::CListNode * n3 = new Common::CListNode();
	n3->Link(n2);
	EXPECT_TRUE(n3->next_ == n2);
	EXPECT_TRUE(n3->prev_ == n1);
	EXPECT_TRUE(n2->next_ == n1);
	EXPECT_TRUE(n2->prev_ == n3);
	EXPECT_TRUE(n1->next_ == n3);
	EXPECT_TRUE(n1->prev_ == n2);

	Common::CListNode * n4 = new Common::CListNode();
	n4->Link(n2);
	EXPECT_TRUE(n4->next_ == n2);
	EXPECT_TRUE(n4->prev_ == n3);
	EXPECT_TRUE(n3->next_ == n4);
	EXPECT_TRUE(n3->prev_ == n1);
	EXPECT_TRUE(n2->next_ == n1);
	EXPECT_TRUE(n2->prev_ == n4);
	EXPECT_TRUE(n1->next_ == n3);
	EXPECT_TRUE(n1->prev_ == n2);

	delete n1;
	delete n2;
	delete n3;
	delete n4;
}

TEST(CListNodeTest, unlink) {
	Common::CListNode * n1 = new Common::CListNode();
	n1->next_ = n1->prev_ = n1;
	Common::CListNode * n2 = new Common::CListNode();
	n2->Link(n1);
	// n2 -> n1
	Common::CListNode * n3 = new Common::CListNode();
	n3->Link(n2);
	// n3 -> n2 -> n1
	Common::CListNode * n4 = new Common::CListNode();
	n4->Link(n2);
	// n3 -> n4 -> n2 -> n1
	n2->Unlink();
	EXPECT_TRUE(n2->next_ == n1);
	EXPECT_TRUE(n2->prev_ == n4);
	// n3 -> n4 -> n1
	EXPECT_TRUE(n3->next_ == n4);
	EXPECT_TRUE(n3->prev_ == n1);
	EXPECT_TRUE(n4->next_ == n1);
	EXPECT_TRUE(n4->prev_ == n3);
	EXPECT_TRUE(n1->next_ == n3);
	EXPECT_TRUE(n1->prev_ == n4);
	n3->Unlink();
	EXPECT_TRUE(n3->next_ == n4);
	EXPECT_TRUE(n3->prev_ == n1);
	// n4 -> n1
	EXPECT_TRUE(n4->next_ == n1);
	EXPECT_TRUE(n4->prev_ == n1);
	EXPECT_TRUE(n1->next_ == n4);
	EXPECT_TRUE(n1->prev_ == n4);
	n1->Unlink();
	EXPECT_TRUE(n1->next_ == n4);
	EXPECT_TRUE(n1->prev_ == n4);
	// n4
	EXPECT_TRUE(n4->next_ == n4);
	EXPECT_TRUE(n4->prev_ == n4);

	delete n1;
	delete n2;
	delete n3;
	delete n4;
}

TEST(CListNodeHeaderTest, ctor) {
	Common::CListNodeHeader * h1 = new Common::CListNodeHeader();
	EXPECT_TRUE(h1->next_ == h1);
	EXPECT_TRUE(h1->prev_ == h1);
	EXPECT_EQ(h1->count_, 0);
	// h1 -> n1
	Common::CListNode * n1 = new Common::CListNode();
	n1->Link(h1->next_);
	EXPECT_TRUE(h1->next_ == n1);
	EXPECT_TRUE(h1->prev_ == n1);
	EXPECT_EQ(++h1->count_, 1);
	// h1 -> n2 -> n1
	Common::CListNode * n2 = new Common::CListNode();
	n2->Link(h1->next_);
	EXPECT_TRUE(h1->next_ == n2);
	EXPECT_TRUE(h1->prev_ == n1);
	EXPECT_TRUE(n2->next_ == n1);
	EXPECT_TRUE(n2->prev_ == h1);
	EXPECT_TRUE(n1->next_ == h1);
	EXPECT_TRUE(n1->prev_ == n2);
	EXPECT_EQ(++h1->count_, 2);
	// h1 -> n2 -> n1 -> n3
	Common::CListNode * n3 = new Common::CListNode();
	n3->Link(h1);
	EXPECT_TRUE(h1->next_ == n2);
	EXPECT_TRUE(h1->prev_ == n3);
	EXPECT_TRUE(n2->next_ == n1);
	EXPECT_TRUE(n2->prev_ == h1);
	EXPECT_TRUE(n1->next_ == n3);
	EXPECT_TRUE(n1->prev_ == n2);
	EXPECT_TRUE(n3->next_ == h1);
	EXPECT_TRUE(n3->prev_ == n1);
	EXPECT_EQ(++h1->count_, 3);
	// h2 -> n2 -> n1 -> n3
	// h1
	Common::CListNodeHeader * h2 = new Common::CListNodeHeader(std::move(*h1));
	EXPECT_TRUE(h2->next_ == n2);
	EXPECT_TRUE(h2->prev_ == n3);
	EXPECT_TRUE(n2->next_ == n1);
	EXPECT_TRUE(n2->prev_ == h2);
	EXPECT_TRUE(n1->next_ == n3);
	EXPECT_TRUE(n1->prev_ == n2);
	EXPECT_TRUE(n3->next_ == h2);
	EXPECT_TRUE(n3->prev_ == n1);
	EXPECT_EQ(h2->count_, 3);
	EXPECT_TRUE(h1->next_ == h1);
	EXPECT_TRUE(h1->prev_ == h1);
	EXPECT_EQ(h1->count_, 0);
	// h3
	// h2 -> n2 -> n1 -> n3
	// h1
	h1->count_ = 10;
	Common::CListNodeHeader * h3 = new Common::CListNodeHeader(std::move(*h1));
	EXPECT_TRUE(h3->next_ == h3);
	EXPECT_TRUE(h3->prev_ == h3);
	EXPECT_EQ(h3->count_, 10);
	EXPECT_TRUE(h1->next_ == h1);
	EXPECT_TRUE(h1->prev_ == h1);
	EXPECT_EQ(h1->count_, 10);

	delete h1;
	delete h2;
	delete h3;
	delete n1;
	delete n2;
	delete n3;
}

TEST(TListNodeTest, ctor) {
	Common::TListNode<i32> * n1 = new Common::TListNode<i32>();
	Common::TListNode<i32> * n2 = new Common::TListNode<i32>(100);
	EXPECT_EQ(n2->data_, 100);

	Common::TListNode<CListTest_MockObject> * m1 = new Common::TListNode<CListTest_MockObject>();
	Common::TListNode<CListTest_MockObject> * m2 = new Common::TListNode<CListTest_MockObject>("123");
	Common::TListNode<CListTest_MockObject> * m3 = new Common::TListNode<CListTest_MockObject>(m2->data_);
	Common::TListNode<CListTest_MockObject> * m4 = new Common::TListNode<CListTest_MockObject>(CListTest_MockObject("456"));
	EXPECT_STREQ(m2->data_.a_.c_str(), "123");
	EXPECT_STREQ(m3->data_.a_.c_str(), "123");
	EXPECT_STREQ(m4->data_.a_.c_str(), "456");

	delete n1;
	delete n2;
	delete m1;
	delete m2;
	delete m3;
	delete m4;
}

TEST(TListIteratorTest, ctor) {
	Common::TListIterator<i32> * it1 = new Common::TListIterator<i32>();
	Common::CListNode * n1 = new Common::CListNode();
	Common::TListIterator<i32> * it2 = new Common::TListIterator<i32>(n1);
	Common::TListIterator<i32> * it3 = new Common::TListIterator<i32>(*it1);
	delete it1;
	delete it2;
	delete it3;
	delete n1;
}

TEST(TListIteratorTest, op) {
	Common::CListNodeHeader * h1 = new Common::CListNodeHeader();
	Common::TListNode<i32> * n1 = new Common::TListNode<i32>(1);
	n1->Link(h1);
	Common::TListNode<i32> * n2 = new Common::TListNode<i32>(2);
	n2->Link(h1);
	Common::TListNode<i32> * n3 = new Common::TListNode<i32>(3);
	n3->Link(h1);
	Common::TListNode<i32> * n4 = new Common::TListNode<i32>(4);
	n4->Link(h1);
	// h1 -> n1 -> n2 -> n3 -> n4
	Common::TListIterator<i32> it(h1);
	Common::TListIterator<i32> it1(n1);
	Common::TListIterator<i32> it2(n2);
	Common::TListIterator<i32> it3(n3);
	Common::TListIterator<i32> it4(n4);
	Common::TListIterator<i32> it5(it);
	Common::TListIterator<i32> it6;
	it6 = it6;
	it6 = it;
	EXPECT_EQ(it5, it6);
	EXPECT_EQ(it5, it);
	EXPECT_EQ(it6, it);
	EXPECT_EQ(++it, it1);
	EXPECT_EQ(*it, 1);
	EXPECT_EQ(it++, it1);
	EXPECT_EQ(*it, 2);
	EXPECT_EQ(it++, it2);
	EXPECT_EQ(*it, 3);
	EXPECT_EQ(++it, it4);
	EXPECT_EQ(*it, 4);
	EXPECT_EQ(it--, it4);
	EXPECT_EQ(*it, 3);
	EXPECT_EQ(it--, it3);
	EXPECT_EQ(*it, 2);
	EXPECT_EQ(--it, it1);
	EXPECT_EQ(*it, 1);
	EXPECT_NE(it1, it2);
	EXPECT_NE(it1, it3);
	EXPECT_NE(it1, it4);
	EXPECT_NE(it2, it3);
	EXPECT_NE(it2, it4);
	EXPECT_NE(it3, it4);
	EXPECT_EQ(it5, it6);
	EXPECT_NE(it5, it);
	EXPECT_NE(it6, it);

	delete n1;
	delete n2;
	delete n3;
	delete n4;
	delete h1;
}

TEST(TListIteratorTest, oop) {
	Common::CListNodeHeader * h1 = new Common::CListNodeHeader();
	Common::TListNode<CListTest_MockObject> * n1 = new Common::TListNode<CListTest_MockObject>("1");
	n1->Link(h1);
	Common::TListNode<CListTest_MockObject> * n2 = new Common::TListNode<CListTest_MockObject>("2");
	n2->Link(h1);
	Common::TListNode<CListTest_MockObject> * n3 = new Common::TListNode<CListTest_MockObject>("3");
	n3->Link(h1);
	Common::TListNode<CListTest_MockObject> * n4 = new Common::TListNode<CListTest_MockObject>("4");
	n4->Link(h1);
	// h1 -> n1 -> n2 -> n3 -> n4
	Common::TListIterator<CListTest_MockObject> it(h1);
	Common::TListIterator<CListTest_MockObject> it1(n1);
	Common::TListIterator<CListTest_MockObject> it2(n2);
	Common::TListIterator<CListTest_MockObject> it3(n3);
	Common::TListIterator<CListTest_MockObject> it4(n4);
	Common::TListIterator<CListTest_MockObject> it5(it);
	Common::TListIterator<CListTest_MockObject> it6;
	it6 = it6;
	it6 = it;
	EXPECT_EQ(it5, it6);
	EXPECT_EQ(it5, it);
	EXPECT_EQ(it6, it);
	EXPECT_EQ(++it, it1);
	EXPECT_STREQ((*it).a_.c_str(), "1");
	EXPECT_STREQ(it->a_.c_str(), "1");
	EXPECT_EQ(it++, it1);
	EXPECT_STREQ((*it).a_.c_str(), "2");
	EXPECT_STREQ(it->a_.c_str(), "2");
	EXPECT_EQ(it++, it2);
	EXPECT_STREQ((*it).a_.c_str(), "3");
	EXPECT_STREQ(it->a_.c_str(), "3");
	EXPECT_EQ(++it, it4);
	EXPECT_STREQ((*it).a_.c_str(), "4");
	EXPECT_STREQ(it->a_.c_str(), "4");
	EXPECT_EQ(it--, it4);
	EXPECT_STREQ((*it).a_.c_str(), "3");
	EXPECT_STREQ(it->a_.c_str(), "3");
	EXPECT_EQ(it--, it3);
	EXPECT_STREQ((*it).a_.c_str(), "2");
	EXPECT_STREQ(it->a_.c_str(), "2");
	EXPECT_EQ(--it, it1);
	EXPECT_STREQ((*it).a_.c_str(), "1");
	EXPECT_STREQ(it->a_.c_str(), "1");
	EXPECT_NE(it1, it2);
	EXPECT_NE(it1, it3);
	EXPECT_NE(it1, it4);
	EXPECT_NE(it2, it3);
	EXPECT_NE(it2, it4);
	EXPECT_NE(it3, it4);
	EXPECT_EQ(it5, it6);
	EXPECT_NE(it5, it);
	EXPECT_NE(it6, it);

	delete n1;
	delete n2;
	delete n3;
	delete n4;
	delete h1;
}

TEST(TListConstIteratorTest, ctor) {
	Common::TListConstIterator<i32> * it1 = new Common::TListConstIterator<i32>();
	Common::CListNode * n1 = new Common::CListNode();
	Common::TListConstIterator<i32> * it2 = new Common::TListConstIterator<i32>(n1);
	Common::TListConstIterator<i32> * it3 = new Common::TListConstIterator<i32>(*it1);
	Common::TListIterator<i32> * it4 = new Common::TListIterator<i32>();
	Common::TListConstIterator<i32> * it5 = new Common::TListConstIterator<i32>(*it4);
	delete it1;
	delete it2;
	delete it3;
	delete it4;
	delete it5;
	delete n1;
}

TEST(TListConstIteratorTest, op) {
	Common::CListNodeHeader * h1 = new Common::CListNodeHeader();
	Common::TListNode<i32> * n1 = new Common::TListNode<i32>(1);
	n1->Link(h1);
	Common::TListNode<i32> * n2 = new Common::TListNode<i32>(2);
	n2->Link(h1);
	Common::TListNode<i32> * n3 = new Common::TListNode<i32>(3);
	n3->Link(h1);
	Common::TListNode<i32> * n4 = new Common::TListNode<i32>(4);
	n4->Link(h1);
	// h1 -> n1 -> n2 -> n3 -> n4
	Common::TListConstIterator<i32> it(h1);
	Common::TListConstIterator<i32> it1(n1);
	Common::TListConstIterator<i32> it2(n2);
	Common::TListConstIterator<i32> it3(n3);
	Common::TListConstIterator<i32> it4(n4);
	Common::TListConstIterator<i32> it5(it);
	Common::TListConstIterator<i32> it6;
	Common::TListIterator<i32> it7(h1);
	Common::TListConstIterator<i32> it8(it7);
	it6 = it6;
	it6 = it;
	EXPECT_EQ(it5, it6);
	EXPECT_EQ(it5, it);
	EXPECT_EQ(it6, it);
	EXPECT_EQ(it8, it);
	EXPECT_EQ(++it, it1);
	EXPECT_EQ(*it, 1);
	EXPECT_EQ(it++, it1);
	EXPECT_EQ(*it, 2);
	EXPECT_EQ(it++, it2);
	EXPECT_EQ(*it, 3);
	EXPECT_EQ(++it, it4);
	EXPECT_EQ(*it, 4);
	EXPECT_EQ(it--, it4);
	EXPECT_EQ(*it, 3);
	EXPECT_EQ(it--, it3);
	EXPECT_EQ(*it, 2);
	EXPECT_EQ(--it, it1);
	EXPECT_EQ(*it, 1);
	EXPECT_NE(it1, it2);
	EXPECT_NE(it1, it3);
	EXPECT_NE(it1, it4);
	EXPECT_NE(it2, it3);
	EXPECT_NE(it2, it4);
	EXPECT_NE(it3, it4);
	EXPECT_EQ(it5, it6);
	EXPECT_NE(it5, it);
	EXPECT_NE(it6, it);
	EXPECT_NE(it8, it);

	delete n1;
	delete n2;
	delete n3;
	delete n4;
	delete h1;
}

TEST(TListConstIteratorTest, oop) {
	Common::CListNodeHeader * h1 = new Common::CListNodeHeader();
	Common::TListNode<CListTest_MockObject> * n1 = new Common::TListNode<CListTest_MockObject>("1");
	n1->Link(h1);
	Common::TListNode<CListTest_MockObject> * n2 = new Common::TListNode<CListTest_MockObject>("2");
	n2->Link(h1);
	Common::TListNode<CListTest_MockObject> * n3 = new Common::TListNode<CListTest_MockObject>("3");
	n3->Link(h1);
	Common::TListNode<CListTest_MockObject> * n4 = new Common::TListNode<CListTest_MockObject>("4");
	n4->Link(h1);
	// h1 -> n1 -> n2 -> n3 -> n4
	Common::TListConstIterator<CListTest_MockObject> it(h1);
	Common::TListConstIterator<CListTest_MockObject> it1(n1);
	Common::TListConstIterator<CListTest_MockObject> it2(n2);
	Common::TListConstIterator<CListTest_MockObject> it3(n3);
	Common::TListConstIterator<CListTest_MockObject> it4(n4);
	Common::TListConstIterator<CListTest_MockObject> it5(it);
	Common::TListConstIterator<CListTest_MockObject> it6;
	Common::TListIterator<CListTest_MockObject> it7(h1);
	Common::TListConstIterator<CListTest_MockObject> it8(it7);
	it6 = it6;
	it6 = it;
	EXPECT_EQ(it5, it6);
	EXPECT_EQ(it5, it);
	EXPECT_EQ(it6, it);
	EXPECT_EQ(it8, it);
	EXPECT_EQ(++it, it1);
	EXPECT_STREQ((*it).a_.c_str(), "1");
	EXPECT_STREQ(it->a_.c_str(), "1");
	EXPECT_EQ(it++, it1);
	EXPECT_STREQ((*it).a_.c_str(), "2");
	EXPECT_STREQ(it->a_.c_str(), "2");
	EXPECT_EQ(it++, it2);
	EXPECT_STREQ((*it).a_.c_str(), "3");
	EXPECT_STREQ(it->a_.c_str(), "3");
	EXPECT_EQ(++it, it4);
	EXPECT_STREQ((*it).a_.c_str(), "4");
	EXPECT_STREQ(it->a_.c_str(), "4");
	EXPECT_EQ(it--, it4);
	EXPECT_STREQ((*it).a_.c_str(), "3");
	EXPECT_STREQ(it->a_.c_str(), "3");
	EXPECT_EQ(it--, it3);
	EXPECT_STREQ((*it).a_.c_str(), "2");
	EXPECT_STREQ(it->a_.c_str(), "2");
	EXPECT_EQ(--it, it1);
	EXPECT_STREQ((*it).a_.c_str(), "1");
	EXPECT_STREQ(it->a_.c_str(), "1");
	EXPECT_NE(it1, it2);
	EXPECT_NE(it1, it3);
	EXPECT_NE(it1, it4);
	EXPECT_NE(it2, it3);
	EXPECT_NE(it2, it4);
	EXPECT_NE(it3, it4);
	EXPECT_EQ(it5, it6);
	EXPECT_NE(it5, it);
	EXPECT_NE(it6, it);
	EXPECT_NE(it8, it);

	delete n1;
	delete n2;
	delete n3;
	delete n4;
	delete h1;
}

class CListTest : public testing::Test {
public:
	virtual void SetUp() {
		i_list_.PushBack(4);
		i_list_.PushBack(5);
		i_list_.EmplaceBack(6);
		i_list_.PushFront(3);
		i_list_.PushFront(2);
		i_list_.EmplaceFront(1);

		CListTest_MockObject tmp("4");
		o_list_.PushBack(tmp);
		o_list_.PushBack(CListTest_MockObject("5"));
		o_list_.EmplaceBack("6");

		CListTest_MockObject tmp2("3");
		o_list_.PushFront(tmp2);
		o_list_.PushFront(CListTest_MockObject("2"));
		o_list_.EmplaceFront("1");
	}
	virtual void TearDown() {
		i_list_.Clear();
		o_list_.Clear();
	}

protected:
	Common::TList<i32> i_list_;
	Common::TList<CListTest_MockObject> o_list_;
};

TEST_F(CListTest, ctor) {
	EXPECT_EQ(i_list_.Empty(), false);
	EXPECT_EQ(i_list_.Size(), 6);

	EXPECT_EQ(o_list_.Empty(), false);
	EXPECT_EQ(o_list_.Size(), 6);

	Common::TList<i32> i_move_list(std::move(i_list_));
	Common::TList<CListTest_MockObject> o_move_list(std::move(o_list_));
	
	EXPECT_EQ(i_list_.Empty(), true);
	EXPECT_EQ(i_list_.Size(), 0);

	EXPECT_EQ(o_list_.Empty(), true);
	EXPECT_EQ(o_list_.Size(), 0);

	EXPECT_EQ(i_move_list.Empty(), false);
	EXPECT_EQ(i_move_list.Size(), 6);

	EXPECT_EQ(o_move_list.Empty(), false);
	EXPECT_EQ(o_move_list.Size(), 6);
}

TEST_F(CListTest, it) {
	Common::TList<i32>::Iterator it = i_list_.Begin();
	EXPECT_EQ(*it, i_list_.Front());
	for (i32 i = 0; i < i_list_.Size(); i++) {
		it++;
	}
	EXPECT_EQ(it, i_list_.End());
	it--;
	EXPECT_EQ(*it, i_list_.Back());
}

TEST_F(CListTest, cit) {
	const Common::TList<i32> * i_move_list = new Common::TList<i32>(std::move(i_list_));
	Common::TList<i32>::ConstIterator it = i_move_list->Begin();
	EXPECT_EQ(*it, i_move_list->Front());
	for (i32 i = 0; i < i_move_list->Size(); i++) {
		it++;
	}
	EXPECT_EQ(it, i_move_list->End());
	it--;
	EXPECT_EQ(*it, i_move_list->Back());
	delete i_move_list;
}

TEST_F(CListTest, oit) {
	Common::TList<CListTest_MockObject>::Iterator it = o_list_.Begin();
	EXPECT_EQ(*it, o_list_.Front());
	for (i32 i = 0; i < o_list_.Size(); i++) {
		it++;
	}
	EXPECT_EQ(it, o_list_.End());
	it--;
	EXPECT_EQ(*it, o_list_.Back());
}

TEST_F(CListTest, ocit) {
	const Common::TList<CListTest_MockObject> * o_move_list = new Common::TList<CListTest_MockObject>(std::move(o_list_));
	Common::TList<CListTest_MockObject>::ConstIterator it = o_move_list->Begin();
	EXPECT_EQ(*it, o_move_list->Front());
	for (i32 i = 0; i < o_move_list->Size(); i++) {
		it++;
	}
	EXPECT_EQ(it, o_move_list->End());
	it--;
	EXPECT_EQ(*it, o_move_list->Back());
	delete o_move_list;
}

TEST_F(CListTest, pop) {
	i_list_.PopBack();
	EXPECT_EQ(i_list_.Size(), 5);
	EXPECT_EQ(i_list_.Back(), 5);
	EXPECT_EQ(i_list_.Front(), 1);

	i_list_.PopFront();
	EXPECT_EQ(i_list_.Size(), 4);
	EXPECT_EQ(i_list_.Back(), 5);
	EXPECT_EQ(i_list_.Front(), 2);
}

TEST_F(CListTest, opop) {
	o_list_.PopBack();
	EXPECT_EQ(o_list_.Size(), 5);
	EXPECT_STREQ(o_list_.Back().a_.c_str(), "5");
	EXPECT_STREQ(o_list_.Front().a_.c_str(), "1");

	o_list_.PopFront();
	EXPECT_EQ(o_list_.Size(), 4);
	EXPECT_STREQ(o_list_.Back().a_.c_str(), "5");
	EXPECT_STREQ(o_list_.Front().a_.c_str(), "2");
}

TEST_F(CListTest, erase) {
	for (Common::TList<i32>::Iterator it = i_list_.Begin(); it != i_list_.End();) {
		if (*it == 2) {
			it = i_list_.Erase(it);
			EXPECT_EQ(i_list_.Size(), 5);
			EXPECT_EQ(*it, 3);
		} else if (*it == 6) {
			it = i_list_.Erase(it);
			EXPECT_EQ(i_list_.Size(), 4);
			EXPECT_EQ(it, i_list_.End());
		} else {
			it++;
		}
	}
}

TEST_F(CListTest, oerase) {
	for (Common::TList<CListTest_MockObject>::Iterator it = o_list_.Begin(); it != o_list_.End();) {
		if (it->a_ == "2") {
			it = o_list_.Erase(it);
			EXPECT_EQ(o_list_.Size(), 5);
			EXPECT_STREQ(it->a_.c_str(), "3");
		} else if (it->a_ == "6") {
			it = o_list_.Erase(it);
			EXPECT_EQ(o_list_.Size(), 4);
			EXPECT_EQ(it, o_list_.End());
		} else {
			it++;
		}
	}
}