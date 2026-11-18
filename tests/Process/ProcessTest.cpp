#include "ProcessTest.hpp"
#include <atomic>
#include <cerrno>
#include <chrono>
#include <csignal>
#include <cstring>
#include <iostream>
#include <thread>
#include <sys/stat.h>

#ifndef PROCESS_TEST_DIR
	#define PROCESS_TEST_DIR ""
#endif

void ProcessTest::SetUp() {}

void ProcessTest::TearDown() {}

TEST_F(ProcessTest, ExecuteProcess) {
	ProcessInfo info{
		PROCESS_TEST_DIR "/test.sh"
	};

	process.SetStartInfo(info);
	process.Start();
	process.Wait();
	EXPECT_EQ(true, process.HasExited());
	EXPECT_EQ(0, process.ExitCode());

	std::string output = process.StandardOutput();
	std::string error = process.StandardError();
	EXPECT_STREQ("This is standard output", output.c_str());
	EXPECT_STREQ("This is standard error", error.c_str());
}

TEST_F(ProcessTest, ExecuteProcessWithEnvironment) {
	ProcessInfo info{ "printf $FOO", false };
	info.SetEnvironment({ "FOO=BAR" });

	process.SetStartInfo(info);
	process.Start();
	process.Wait();
	EXPECT_EQ(true, process.HasExited());
	EXPECT_EQ(0, process.ExitCode());

	EXPECT_STREQ("BAR", process.StandardOutput().c_str());
}

TEST_F(ProcessTest, TimeoutProcess) {
	ProcessInfo info;
	info.commandLine = "sleep 5";

	process.SetStartInfo(info);
	process.Start();
	bool ret = process.WaitFor(1000);
	EXPECT_EQ(false, ret);
	EXPECT_EQ(false, process.HasExited());
	EXPECT_EQ(-1, process.ExitCode());
}

TEST_F(ProcessTest, eventHandler) {
	void (*onExited)(Process * self) = [](Process *self) {
		std::cout << "onExited: " << self->id << std::endl;
	};
	process.onExited = onExited;
	ProcessInfo info1{ "printf hoge" };

	ProcessClass process2;
	process2.onExited = onExited;
	ProcessInfo info2{ "printf fuga" };

	process.SetStartInfo(info1);
	process.Start();
	process2.SetStartInfo(info2);
	process2.Start();

	std::thread thread1{ [&]() {
		process.Wait();
	} };
	std::thread thread2{ [&]() {
		process2.Wait();
	} };

	thread1.join();
	thread2.join();

	EXPECT_EQ(true, process.HasExited());
	EXPECT_EQ(0, process.ExitCode());
	EXPECT_STREQ("hoge", process.StandardOutput().c_str());

	EXPECT_EQ(true, process2.HasExited());
	EXPECT_EQ(0, process2.ExitCode());
	EXPECT_STREQ("fuga", process2.StandardOutput().c_str());
}

TEST_F(ProcessTest, OutputProperties) {
	ProcessInfo info{ "python3 " PROCESS_TEST_DIR "/proc.py " PROCESS_TEST_DIR " ProcessUnitTest", true };
	process.SetStartInfo(info);
	process.Start();
	process.Wait();

	process.PrintSummary();

	ProcessInfo info2{ "python3 " PROCESS_TEST_DIR "/proc.py " PROCESS_TEST_DIR " ProcessUnitTest", true };
	ProcessClass process2;
	process2.SetStartInfo(info2);
	process2.Start();
	process2.WaitFor(10 * 1000);

	process2.PrintSummary();
}

// ---------------------------------------------------------------------------
// 回帰テスト
// ---------------------------------------------------------------------------

namespace {
ProcessStartInfo MakeInfo(const char *commandLine) {
	ProcessStartInfo info{};
	info.commandLine = commandLine;
	return info;
}
}  // namespace

// WaitFor が true を返した直後に終了コードが確定していること
TEST(ProcessRegressionTest, ExitCodeAvailableRightAfterWaitFor) {
	ProcessStartInfo info = MakeInfo("exit 3");
	Process process;
	Process_Execute(&process, &info);
	ASSERT_TRUE(Process_WaitFor(&process, 5000));
	EXPECT_TRUE(Process_HasExited(&process));
	EXPECT_EQ(3, Process_ExitCode(&process));
	EXPECT_LE(Process_StartTime(&process), Process_ExitTime(&process));
	Process_Destroy(&process);
}

// 即座に終了する子プロセスを大量に同時起動しても、終了を取りこぼさないこと
// (以前は fork 直後の終了や SIGCHLD の合流で WaitFor がタイムアウトしていた)
TEST(ProcessRegressionTest, ManyShortLivedProcesses) {
	constexpr int numProcesses = 32;
	ProcessStartInfo info = MakeInfo("exit 0");
	std::vector<Process> processes(numProcesses);
	for (auto &process : processes) {
		Process_Execute(&process, &info);
	}
	for (auto &process : processes) {
		EXPECT_TRUE(Process_WaitFor(&process, 10 * 1000));
		EXPECT_EQ(0, Process_ExitCode(&process));
	}
	for (auto &process : processes) {
		Process_Destroy(&process);
	}
}

// ライブラリがアプリケーションの SIGCHLD ハンドラを書き換えないこと
TEST(ProcessRegressionTest, DoesNotOverrideSigchldHandler) {
	struct sigaction before {};
	sigaction(SIGCHLD, nullptr, &before);
	ProcessStartInfo info = MakeInfo("exit 0");
	Process process;
	Process_Execute(&process, &info);
	Process_Wait(&process);
	struct sigaction after {};
	sigaction(SIGCHLD, nullptr, &after);
	EXPECT_EQ((void *)before.sa_handler, (void *)after.sa_handler);
	Process_Destroy(&process);
}

// ワークスペースはプロセスごとに固有で、所有者のみアクセス可能であること
TEST(ProcessRegressionTest, WorkspaceIsUniqueAndPrivate) {
	Process p1;
	Process p2;
	Process_Init(&p1, nullptr);
	Process_Init(&p2, nullptr);
	ASSERT_NE('\0', p1.workspace[0]);
	ASSERT_NE('\0', p2.workspace[0]);
	EXPECT_STRNE(p1.workspace, p2.workspace);
	struct stat st {};
	ASSERT_EQ(0, stat(p1.workspace, &st));
	EXPECT_EQ(0700u, st.st_mode & 0777);
	std::string path = p1.workspace;
	Process_Destroy(&p1);
	Process_Destroy(&p2);
	EXPECT_NE(0, stat(path.c_str(), &st));	// 破棄時に削除されること
}

// 実行中に Destroy した場合、子プロセスを終了・回収してから破棄すること
TEST(ProcessRegressionTest, DestroyWhileRunningReapsChild) {
	ProcessStartInfo info = MakeInfo("sleep 30");
	Process process;
	Process_Execute(&process, &info);
	pid_t pid = Process_Id(&process);
	ASSERT_GT(pid, 0);
	auto start = std::chrono::steady_clock::now();
	Process_Destroy(&process);
	EXPECT_LT(std::chrono::steady_clock::now() - start, std::chrono::seconds(5));
	errno = 0;
	EXPECT_EQ(-1, kill(pid, 0));	// ゾンビも残っていないこと
	EXPECT_EQ(ESRCH, errno);
}

// Wait から戻った時点で onExited の呼び出しが完了していること
TEST(ProcessRegressionTest, OnExitedCompletesBeforeWaitReturns) {
	static std::atomic<int> calledCount{ 0 };
	static std::atomic<int> exitCodeInCallback{ -100 };
	calledCount = 0;
	ProcessStartInfo info = MakeInfo("exit 5");
	Process process;
	Process_Init(&process, &info);
	process.onExited = [](Process *self) {
		exitCodeInCallback = Process_ExitCode(self);	// コールバック内でも終了コードが取れる
		calledCount++;
	};
	Process_Start(&process);
	Process_Wait(&process);
	EXPECT_EQ(1, calledCount.load());
	EXPECT_EQ(5, exitCodeInCallback.load());
	Process_Destroy(&process);
}

// 起動していないインスタンスに対する問い合わせが安全であること
TEST(ProcessRegressionTest, NotStartedProcess) {
	Process process;
	Process_Init(&process, nullptr);
	EXPECT_EQ(-1, Process_Id(&process));
	EXPECT_FALSE(Process_HasExited(&process));
	EXPECT_FALSE(Process_WaitFor(&process, 10));
	ProcessOutput output;
	memset(&output, 0xAB, sizeof(output));
	Process_StandardOutput(&process, &output);
	EXPECT_EQ(0u, output.size);
	Process_Destroy(&process);
}
