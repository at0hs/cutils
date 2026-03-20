#pragma once

#include "Thread/BackGroundTask.h"
#include "gtest/gtest.h"
#include <condition_variable>
#include <mutex>
#include <unistd.h>

class BackGroundTaskTest : public ::testing::Test {
protected:
	static bool isReady;
	static std::mutex mutex;
	static std::condition_variable cond;

	static BackGroundTaskStatus worker(BackGroundTask_t *o, void *s);
	static void completed(BackGroundTask_t *o, BackGroundTaskStatus exitStatus);
	static void progressChanged(BackGroundTask_t *o, BackGroundTaskProgress progress);

	BackGroundTask_t task;
	virtual void SetUp();
	virtual void TearDown();

	bool TimedWait(int sec);
};
