#include "BackGroundTaskTest.hpp"
#include <chrono>
#include <thread>

bool BackGroundTaskTest::isReady = false;
int BackGroundTaskTest::pendingEvents = 0;
std::mutex BackGroundTaskTest::mutex;
std::condition_variable BackGroundTaskTest::cond;

BackGroundTaskStatus BackGroundTaskTest::worker(BackGroundTask_t *o, void *s) {
	return BACK_GROUND_TASK_EXITED;
}

void BackGroundTaskTest::completed(BackGroundTask_t *o, BackGroundTaskStatus exitStatus) {
	// printf("completed=%d\n", exitStatus);
	{
		std::lock_guard<std::mutex> lock(mutex);
		pendingEvents++;
	}
	cond.notify_one();
}

void BackGroundTaskTest::progressChanged(BackGroundTask_t *o, BackGroundTaskProgress progress) {
	// printf("Report progress=%d\n", progress);
	{
		std::lock_guard<std::mutex> lock(mutex);
		pendingEvents++;
	}
	cond.notify_one();
}

void BackGroundTaskTest::SetUp() {
	isReady = false;
	pendingEvents = 0;
	BackGroundTaskWork_t w = { .func = worker, .sender = nullptr };
	BackGroundTask_Init(&task, &w);
}

void BackGroundTaskTest::TearDown() {
	BackGroundTask_Destroy(&task);
}

bool BackGroundTaskTest::TimedWait(int sec) {
	// 通知が待機開始より先に来ても取りこぼさないよう、イベント数を述語にして待つ
	std::unique_lock<std::mutex> uniq_lk(mutex);
	if (!cond.wait_for(uniq_lk, std::chrono::seconds(sec), [] { return pendingEvents > 0; })) {
		return false;
	}
	pendingEvents--;
	return true;
}

TEST_F(BackGroundTaskTest, NormalFinish) {
	task.work.func = [](BackGroundTask_t *o, void *s) {
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
		return BACK_GROUND_TASK_EXITED;
	};
	task.eventHandlers.finished = completed;
	BackGroundTask_Run(&task);
	ASSERT_TRUE(TimedWait(5));
	EXPECT_EQ(BACK_GROUND_TASK_EXITED, task.exitStatus);
}

TEST_F(BackGroundTaskTest, ProgressReport) {
	task.work.func = [](BackGroundTask_t *o, void *s) {
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		int prog = 0;
		for (int i = 0; i < 5; i++) {
			BackGroundTask_ReportsProgress(o, prog += 20);
			std::this_thread::sleep_for(std::chrono::milliseconds(300));
		}
		return BACK_GROUND_TASK_EXITED;
	};
	task.eventHandlers.finished = completed;
	task.eventHandlers.progressChanged = progressChanged;
	BackGroundTask_Run(&task);
	for (int i = 0; i < 5; i++) {
		ASSERT_TRUE(TimedWait(5));
		EXPECT_EQ((i + 1) * 20, task.progress);
	}
	ASSERT_TRUE(TimedWait(5));
	EXPECT_EQ(BACK_GROUND_TASK_EXITED, task.exitStatus);
}

TEST_F(BackGroundTaskTest, setup) {
	task.work.func = [](BackGroundTask_t *o, void *s) {
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		return BACK_GROUND_TASK_EXITED;
	};
	task.eventHandlers.finished = completed;
	task.eventHandlers.setup = [](BackGroundTask_t *o) {
		{
			std::lock_guard<std::mutex> lock(BackGroundTaskTest::mutex);
			BackGroundTaskTest::isReady = true;
			BackGroundTaskTest::pendingEvents++;
		}
		BackGroundTaskTest::cond.notify_one();
	};
	BackGroundTask_Run(&task);
	ASSERT_TRUE(TimedWait(5));
	ASSERT_EQ(true, isReady);
	ASSERT_TRUE(TimedWait(5));
	EXPECT_EQ(BACK_GROUND_TASK_EXITED, task.exitStatus);
}

TEST_F(BackGroundTaskTest, WorkCancellation) {
	task.work.func = [](BackGroundTask_t *o, void *s) {
		while (o->cancellationPending == false) {
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
		// printf("Canceled!!\n");
		return BACK_GROUND_TASK_CANCELED;
	};
	task.eventHandlers.finished = completed;
	BackGroundTask_Run(&task);
	BackGroundTask_Cancel(&task);
	ASSERT_TRUE(TimedWait(5));
	EXPECT_EQ(BACK_GROUND_TASK_CANCELED, task.exitStatus);
}

TEST_F(BackGroundTaskTest, AbortTask) {
	task.work.func = [](BackGroundTask_t *o, void *s) {
		for (int i = 0; i < 100; i++) {
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
		return BACK_GROUND_TASK_EXITED;
	};
	task.eventHandlers.finished = completed;
	BackGroundTask_Run(&task);
	usleep(1000 * 200);
	BackGroundTask_Abort(&task);
	ASSERT_TRUE(TimedWait(5));
	EXPECT_EQ(BACK_GROUND_TASK_ABORTED, task.exitStatus);
}
