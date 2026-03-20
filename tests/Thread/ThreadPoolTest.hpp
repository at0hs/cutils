#pragma once

#include "gtest/gtest.h"
#include "Thread/ThreadPool.h"

class ThreadPoolTest : public ::testing::Test {
private:
	bool destroyed = false;

protected:
	ThreadPool_t pool;
	virtual void SetUp();
	virtual void TearDown();
	int Push(const ThreadPoolTask_t *task);
	int PushTasks(const ThreadPoolTask_t *tasks[], uint64_t numTasks);
	void Destroy(bool wait);
};
