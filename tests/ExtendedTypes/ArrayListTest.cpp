#include "ArrayListTest.hpp"
#include <vector>

TEST_F(ArrayListTest, properties) {
	int a = 1, b = 2;
	list.Add(a);
	list.Add(b);
	EXPECT_EQ(2, list.Length());
	EXPECT_EQ(64, list.Capacity());
}

TEST_F(ArrayListTest, AccessElement) {
	int a = 1;
	int b = 2;
	list.Add(a);
	list.Add(b);
	EXPECT_EQ(a, list.At(0));
	EXPECT_EQ(b, list.At(1));
	EXPECT_EQ(nullptr, ArrayList_At(&list, 2));
}

TEST_F(ArrayListTest, overCapacity) {
	list.Resize(2);
	EXPECT_EQ(2, list.Capacity());
	list.Add(1);
	list.Add(2);
	list.Add(3);
	EXPECT_GT(list.Capacity(), 2);
	EXPECT_EQ(1, list.At(0));
	EXPECT_EQ(2, list.At(1));
	EXPECT_EQ(3, list.At(2));
}

TEST_F(ArrayListTest, Empty) {
	EXPECT_EQ(true, list.IsEmpty());
	int a = 1;
	list.Add(a);
	EXPECT_EQ(false, list.IsEmpty());
}

TEST_F(ArrayListTest, AddReference) {
	int a = 1;
	list.AddReference(a);
	EXPECT_EQ(a, list.At(0));
}

TEST_F(ArrayListTest, AddRange) {
	int a = 1, b = 2, c = 3;
	std::vector<int> nums{ 4, 5, 6 };
	list.Add(a);
	list.Add(b);
	list.Add(c);
	list.AddRange(nums);
	ASSERT_EQ(6, list.Length());
	EXPECT_EQ(a, list.At(0));
	EXPECT_EQ(b, list.At(1));
	EXPECT_EQ(c, list.At(2));
	EXPECT_EQ(4, list.At(3));
	EXPECT_EQ(5, list.At(4));
	EXPECT_EQ(6, list.At(5));
}

TEST_F(ArrayListTest, Append) {
	list.Add(1);
	list.Add(2);
	ArrayList test;
	int a = 3, b = 4;
	ArrayList_Init(&test, nullptr, sizeof(int));
	ArrayList_Add(&test, &a);
	ArrayList_Add(&test, &b);
	list.Append(test);
	ASSERT_EQ(4, list.Length());
	EXPECT_EQ(3, list.At(2));
	EXPECT_EQ(4, list.At(3));
	ArrayList_Destroy(&test);
}

TEST_F(ArrayListTest, Insert) {
	list.Add(1);
	list.Add(2);
	list.Insert(1, 3);
	ASSERT_EQ(3, list.Length());
	EXPECT_EQ(1, list.At(0));
	EXPECT_EQ(3, list.At(1));
	EXPECT_EQ(2, list.At(2));
}

TEST_F(ArrayListTest, InsertOf) {
	list.Add(1);
	list.Add(2);
	EXPECT_EQ(1, list.IndexOf(2));
	EXPECT_EQ(-1, list.IndexOf(0));
}

TEST_F(ArrayListTest, Contains) {
	list.Add(1);
	list.Add(2);
	EXPECT_EQ(true, list.Contains(1));
	EXPECT_EQ(false, list.Contains(0));
}

TEST_F(ArrayListTest, Remove) {
	list.Add(1);
	list.Add(2);
	EXPECT_EQ(false, list.Remove(0));
	list.Remove(1);
	ASSERT_EQ(1, list.Length());
	ASSERT_EQ(2, list.At(0));
	list.Remove(2);
	EXPECT_EQ(true, list.IsEmpty());
	EXPECT_EQ(false, list.Remove(0));
}

TEST_F(ArrayListTest, RemoveAt) {
	list.Add(1);
	list.Add(2);
	list.RemoveAt(0);
	ASSERT_EQ(1, list.Length());
	ASSERT_EQ(2, list.At(0));
	list.RemoveAt(0);
	ASSERT_EQ(true, list.IsEmpty());
	EXPECT_EQ(false, list.RemoveAt(0));
}

TEST_F(ArrayListTest, Clear) {
	list.Add(1);
	list.Add(2);
	list.Clear();
	ASSERT_EQ(true, list.IsEmpty());
}

TEST_F(ArrayListTest, ForEach) {
	list.Add(1);
	list.Add(2);
	list.ForEach([](uint64_t i, const void *o) {
		std::cout << "index=" << i << ", value=" << *(const int *)o << std::endl;
	});
}

TEST_F(ArrayListTest, NullSafety) {
	ArrayList_Init(nullptr, nullptr, sizeof(int));
	ArrayList_Destroy(nullptr);
	EXPECT_EQ(0, ArrayList_Capacity(nullptr));
	EXPECT_EQ(0, ArrayList_Length(nullptr));
	EXPECT_EQ(true, ArrayList_IsEmpty(nullptr));
	ArrayList_Resize(nullptr, 0);
	EXPECT_EQ(nullptr, ArrayList_At(nullptr, 0));
	ArrayList_Add(nullptr, nullptr);
	ArrayList_AddReference(nullptr, nullptr);
	ArrayList_AddRange(nullptr, nullptr, 0);
	ArrayList_Append(nullptr, nullptr);
	ArrayList_Insert(nullptr, 0, nullptr);
	EXPECT_EQ(-1, ArrayList_IndexOf(nullptr, nullptr));
	EXPECT_EQ(false, ArrayList_Contains(nullptr, nullptr));
	EXPECT_EQ(false, ArrayList_Remove(nullptr, nullptr));
	EXPECT_EQ(false, ArrayList_RemoveAt(nullptr, 0));
	ArrayList_Clear(nullptr);
	ArrayList_ForEach(nullptr, nullptr);
}
