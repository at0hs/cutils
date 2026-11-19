#include "ThreadPoolTest.hpp"
#include <chrono>
#include <thread>
#include <atomic>
#include <cstdio>

void ThreadPoolTest::SetUp() {
	ASSERT_EQ(THREAD_POOL_SUCCESS, ThreadPool_Init(&pool, 0));
}

void ThreadPoolTest::TearDown() {
	if (!destroyed)
		ThreadPool_Destroy(&pool, false);
}

ThreadPoolReturnCode ThreadPoolTest::Push(const ThreadPoolTask_t *task) {
	return ThreadPool_Push(&pool, task);
}

ThreadPoolReturnCode ThreadPoolTest::PushTasks(const ThreadPoolTask_t *tasks[], uint64_t numTasks) {
	return ThreadPool_PushTasks(&pool, tasks, numTasks);
}

void ThreadPoolTest::Destroy(bool wait) {
	ThreadPool_Destroy(&pool, wait);
	destroyed = true;
}

void ThreadPoolTest::Reinit(uint16_t numThreads) {
	Destroy(true);
	ASSERT_EQ(THREAD_POOL_SUCCESS, ThreadPool_Init(&pool, numThreads));
	destroyed = false;
}

TEST_F(ThreadPoolTest, Push) {
	ThreadPoolTask_t task;
	int testNum = 10;
	static bool called = false;
	static int taskArg = 0;
	task.function = [](void *arg) {
		called = true;
		taskArg = *(int *)arg;
		};
	task.arg = &testNum;
	ASSERT_EQ(THREAD_POOL_SUCCESS, Push(&task));

	Destroy(true);

	EXPECT_EQ(true, called);
	EXPECT_EQ(testNum, taskArg);
}

TEST_F(ThreadPoolTest, PushTasks) {
	ThreadPoolTask_t task1, task2, task3;
	static bool task1Executed = false;
	static bool task2Executed = false;
	static bool task3Executed = false;
	task1.function = [](void *arg) {
		task1Executed = true;
		};
	task2.function = [](void *arg) {
		task2Executed = true;
		};
	task3.function = [](void *arg) {
		task3Executed = true;
		};
	const ThreadPoolTask_t *tasks[] = {&task1, &task2, &task3};
	ASSERT_EQ(THREAD_POOL_SUCCESS, PushTasks(tasks, 3));
	Destroy(true);

	ASSERT_EQ(true, task1Executed);
	ASSERT_EQ(true, task2Executed);
	ASSERT_EQ(true, task3Executed);
}

TEST_F(ThreadPoolTest, HeavyProcessing) {
	ThreadPoolTask_t task1, task2, task3;
	static bool task1Executed = false;
	static bool task2Executed = false;
	static bool task3Executed = false;
	task1.function = [](void *arg) {
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		task1Executed = true;
		};
	task2.function = [](void *arg) {
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		task2Executed = true;
		};
	task3.function = [](void *arg) {
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		task3Executed = true;
		};

	ASSERT_EQ(THREAD_POOL_SUCCESS, Push(&task1));
	ASSERT_EQ(THREAD_POOL_SUCCESS, Push(&task2));
	ASSERT_EQ(THREAD_POOL_SUCCESS, Push(&task3));
	Destroy(true);
	EXPECT_EQ(true, task1Executed);
	EXPECT_EQ(true, task2Executed);
	EXPECT_EQ(true, task3Executed);
}

TEST_F(ThreadPoolTest, InvalidArguments) {
	ThreadPoolTask_t noFunc = {nullptr, nullptr};
	ThreadPoolTask_t valid = {[](void *) {}, nullptr};
	EXPECT_EQ(THREAD_POOL_INVALID_ARG, ThreadPool_Init(nullptr, 1));
	EXPECT_EQ(THREAD_POOL_INVALID_ARG, ThreadPool_Push(nullptr, &valid));
	EXPECT_EQ(THREAD_POOL_INVALID_ARG, Push(nullptr));
	EXPECT_EQ(THREAD_POOL_INVALID_ARG, Push(&noFunc));
	EXPECT_EQ(THREAD_POOL_INVALID_ARG, PushTasks(nullptr, 1));
	const ThreadPoolTask_t *tasks[] = {&valid};
	EXPECT_EQ(THREAD_POOL_INVALID_ARG, PushTasks(tasks, 0));
	EXPECT_EQ(0u, ThreadPool_GetNumTasks(nullptr));
	ThreadPool_Destroy(nullptr, true);  // クラッシュしないこと
}

TEST_F(ThreadPoolTest, UseAfterDestroy) {
	ThreadPoolTask_t valid = {[](void *) {}, nullptr};
	const ThreadPoolTask_t *tasks[] = {&valid};
	Destroy(true);
	EXPECT_EQ(THREAD_POOL_NOT_INITIALIZED, Push(&valid));
	EXPECT_EQ(THREAD_POOL_NOT_INITIALIZED, PushTasks(tasks, 1));
	EXPECT_EQ(0u, ThreadPool_GetNumTasks(&pool));
	ThreadPool_Destroy(&pool, true);  // 二重 Destroy は無視される
}

// 1 件でも不正なタスクがあれば 1 件も追加されない
TEST_F(ThreadPoolTest, PushTasksIsAllOrNothing) {
	static std::atomic<int> executed{0};
	executed = 0;
	ThreadPoolTask_t valid = {[](void *) { executed++; }, nullptr};
	ThreadPoolTask_t invalid = {nullptr, nullptr};
	const ThreadPoolTask_t *tasks[] = {&valid, &invalid, &valid};
	EXPECT_EQ(THREAD_POOL_INVALID_ARG, PushTasks(tasks, 3));
	const ThreadPoolTask_t *withNull[] = {&valid, nullptr};
	EXPECT_EQ(THREAD_POOL_INVALID_ARG, PushTasks(withNull, 2));
	Destroy(true);
	EXPECT_EQ(0, executed.load());
}

namespace {
std::atomic<bool> blockerStarted{false};
std::atomic<bool> blockerRelease{false};
std::atomic<int> counted{0};

void Blocker(void *) {
	blockerStarted = true;
	while (!blockerRelease)
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
}

void Count(void *) { counted++; }

// ワーカー 1 本を Blocker で塞ぎ、後続 3 件をキューに滞留させる
void FillQueue(ThreadPool_t *pool) {
	blockerStarted = false;
	blockerRelease = false;
	counted = 0;
	ThreadPoolTask_t blocker = {Blocker, nullptr};
	ThreadPoolTask_t count = {Count, nullptr};
	ASSERT_EQ(THREAD_POOL_SUCCESS, ThreadPool_Push(pool, &blocker));
	while (!blockerStarted)
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	const ThreadPoolTask_t *tasks[] = {&count, &count, &count};
	ASSERT_EQ(THREAD_POOL_SUCCESS, ThreadPool_PushTasks(pool, tasks, 3));
}
}  // namespace

TEST_F(ThreadPoolTest, GetNumTasksAndWaitDestroy) {
	Reinit(1);
	FillQueue(&pool);
	EXPECT_EQ(3u, ThreadPool_GetNumTasks(&pool));
	std::thread releaser([] {
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
		blockerRelease = true;
	});
	Destroy(true);
	releaser.join();
	EXPECT_EQ(3, counted.load());
}

TEST_F(ThreadPoolTest, DestroyWithoutWaitDiscardsPendingTasks) {
	Reinit(1);
	FillQueue(&pool);
	std::thread releaser([] {
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
		blockerRelease = true;
	});
	Destroy(false);
	releaser.join();
	EXPECT_EQ(0, counted.load());
}
