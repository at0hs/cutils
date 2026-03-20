#include "LinkedListTest.hpp"

// ------------------------------------------------------------------ Add / At

TEST_F(LinkedListTest, Add) {
	list.Add(1);
	list.Add(2);
	list.Add(3);
	ASSERT_EQ(3, list.Count());
	EXPECT_EQ(1, list.At(0));
	EXPECT_EQ(2, list.At(1));
	EXPECT_EQ(3, list.At(2));
}

TEST_F(LinkedListTest, At_OutOfRange) {
	list.Add(1);
	EXPECT_EQ(nullptr, LinkedList_At(&list, 1));
	EXPECT_EQ(nullptr, LinkedList_At(&list, 100));
}

// --------------------------------------------------------------- IsEmpty / Count

TEST_F(LinkedListTest, IsEmpty) {
	EXPECT_TRUE(list.IsEmpty());
	list.Add(1);
	EXPECT_FALSE(list.IsEmpty());
}

TEST_F(LinkedListTest, Count) {
	EXPECT_EQ(0, list.Count());
	list.Add(1);
	EXPECT_EQ(1, list.Count());
	list.Add(2);
	EXPECT_EQ(2, list.Count());
}

// --------------------------------------------------------------- Contains

TEST_F(LinkedListTest, Contains) {
	list.Add(10);
	list.Add(20);
	EXPECT_TRUE(list.Contains(10));
	EXPECT_TRUE(list.Contains(20));
	EXPECT_FALSE(list.Contains(99));
}

// --------------------------------------------------------------- Remove

TEST_F(LinkedListTest, Remove_Head) {
	list.Add(1);
	list.Add(2);
	list.Add(3);
	EXPECT_TRUE(list.Remove(1));
	ASSERT_EQ(2, list.Count());
	EXPECT_EQ(2, list.At(0));
	EXPECT_EQ(3, list.At(1));
}

TEST_F(LinkedListTest, Remove_Middle) {
	list.Add(1);
	list.Add(2);
	list.Add(3);
	EXPECT_TRUE(list.Remove(2));
	ASSERT_EQ(2, list.Count());
	EXPECT_EQ(1, list.At(0));
	EXPECT_EQ(3, list.At(1));
}

TEST_F(LinkedListTest, Remove_Tail) {
	list.Add(1);
	list.Add(2);
	list.Add(3);
	EXPECT_TRUE(list.Remove(3));
	ASSERT_EQ(2, list.Count());
	EXPECT_EQ(1, list.At(0));
	EXPECT_EQ(2, list.At(1));
}

TEST_F(LinkedListTest, Remove_NotFound) {
	list.Add(1);
	EXPECT_FALSE(list.Remove(99));
	EXPECT_EQ(1, list.Count());
}

TEST_F(LinkedListTest, Remove_OnEmpty) {
	EXPECT_FALSE(list.Remove(1));
}

// --------------------------------------------------------------- RemoveAt

TEST_F(LinkedListTest, RemoveAt_Head) {
	list.Add(1);
	list.Add(2);
	list.Add(3);
	EXPECT_TRUE(list.RemoveAt(0));
	ASSERT_EQ(2, list.Count());
	EXPECT_EQ(2, list.At(0));
	EXPECT_EQ(3, list.At(1));
}

TEST_F(LinkedListTest, RemoveAt_Middle) {
	list.Add(1);
	list.Add(2);
	list.Add(3);
	EXPECT_TRUE(list.RemoveAt(1));
	ASSERT_EQ(2, list.Count());
	EXPECT_EQ(1, list.At(0));
	EXPECT_EQ(3, list.At(1));
}

TEST_F(LinkedListTest, RemoveAt_Tail) {
	list.Add(1);
	list.Add(2);
	list.Add(3);
	EXPECT_TRUE(list.RemoveAt(2));
	ASSERT_EQ(2, list.Count());
	EXPECT_EQ(1, list.At(0));
	EXPECT_EQ(2, list.At(1));
}

TEST_F(LinkedListTest, RemoveAt_OutOfRange) {
	list.Add(1);
	EXPECT_FALSE(list.RemoveAt(1));
	EXPECT_FALSE(list.RemoveAt(100));
	EXPECT_EQ(1, list.Count());
}

// --------------------------------------------------------------- Clear

TEST_F(LinkedListTest, Clear) {
	list.Add(1);
	list.Add(2);
	list.Clear();
	EXPECT_TRUE(list.IsEmpty());
	EXPECT_EQ(0, list.Count());
}

TEST_F(LinkedListTest, Clear_ThenAdd) {
	list.Add(1);
	list.Clear();
	list.Add(2);
	ASSERT_EQ(1, list.Count());
	EXPECT_EQ(2, list.At(0));
}

// --------------------------------------------------------------- Foreach

TEST_F(LinkedListTest, Foreach) {
	list.Add(10);
	list.Add(20);
	list.Add(30);

	std::vector<int> result;
	const LinkedListNode_t *node;
	LinkedList_Foreach(&list, node) {
		result.push_back(*static_cast<const int *>(LinkedList_GetObject(node)));
	}

	ASSERT_EQ(3, result.size());
	EXPECT_EQ(10, result[0]);
	EXPECT_EQ(20, result[1]);
	EXPECT_EQ(30, result[2]);
}

// --------------------------------------------------------------- NullSafety

TEST_F(LinkedListTest, NullSafety) {
	LinkedList_Init(nullptr, nullptr, sizeof(int));
	LinkedList_Destroy(nullptr);
	LinkedList_Add(nullptr, nullptr);
	EXPECT_EQ(0, LinkedList_Count(nullptr));
	EXPECT_TRUE(LinkedList_IsEmpty(nullptr));
	EXPECT_EQ(nullptr, LinkedList_At(nullptr, 0));
	EXPECT_FALSE(LinkedList_Contains(nullptr, nullptr));
	EXPECT_FALSE(LinkedList_Remove(nullptr, nullptr));
	EXPECT_FALSE(LinkedList_RemoveAt(nullptr, 0));
	LinkedList_Clear(nullptr);
}
