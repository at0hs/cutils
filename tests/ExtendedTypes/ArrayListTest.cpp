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

namespace {
void *FailingAllocate(size_t) {
	return nullptr;
}
}  // namespace

TEST(ArrayListRegressionTest, AppendSelfDoublesElements) {
	ArrayList list;
	ArrayList_Init(&list, nullptr, sizeof(int));
	for (int i = 0; i < 3; i++) {
		ArrayList_Add(&list, &i);
	}
	ArrayList_Append(&list, &list);	// 以前は無限ループしていた
	ASSERT_EQ(6u, ArrayList_Length(&list));
	for (int i = 0; i < 6; i++) {
		EXPECT_EQ(i % 3, *(const int *)ArrayList_At(&list, i));
	}
	ArrayList_Destroy(&list);
}

TEST(ArrayListRegressionTest, ZeroObjectSizeIsSafe) {
	ArrayList list;
	memset(&list, 0xAB, sizeof(list));	// 未初期化相当のゴミ
	ArrayList_Init(&list, nullptr, 0);
	int value = 1;
	ArrayList_Add(&list, &value);
	EXPECT_EQ(0u, ArrayList_Length(&list));
	ArrayList_Destroy(&list);	// ゴミポインタを free しないこと
}

TEST(ArrayListRegressionTest, InsertWithAllocationFailureKeepsList) {
	MemoryAllocator_t allocator;
	MemoryAllocator_Init(&allocator, nullptr, nullptr);
	ArrayList list;
	ArrayList_Init(&list, &allocator, sizeof(int));
	int a = 1, b = 2, c = 3;
	ArrayList_Add(&list, &a);
	ArrayList_Add(&list, &b);
	list.allocator.allocate = FailingAllocate;	// 以降の確保は失敗させる
	ArrayList_Insert(&list, 0, &c);
	ArrayList_Add(&list, &c);
	list.allocator.allocate = malloc;
	ASSERT_EQ(2u, ArrayList_Length(&list));
	EXPECT_EQ(a, *(const int *)ArrayList_At(&list, 0));
	EXPECT_EQ(b, *(const int *)ArrayList_At(&list, 1));
	ArrayList_Destroy(&list);
}

TEST(ArrayListRegressionTest, GrowsBeyondInitialCapacity) {
	ArrayList list;
	ArrayList_Init(&list, nullptr, sizeof(int));
	for (int i = 0; i < 1000; i++) {
		ArrayList_Add(&list, &i);
	}
	ASSERT_EQ(1000u, ArrayList_Length(&list));
	EXPECT_EQ(999, *(const int *)ArrayList_At(&list, 999));
	ArrayList_Resize(&list, 10);	// 要素を切り捨てる縮小は無視される
	EXPECT_EQ(1000u, ArrayList_Length(&list));
	EXPECT_LE(1000u, ArrayList_Capacity(&list));
	ArrayList_Destroy(&list);
}
