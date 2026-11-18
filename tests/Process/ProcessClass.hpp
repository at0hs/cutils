#include <string>
#include <vector>
#include "Process/Process.h"

struct ProcessInfo {
	std::string commandLine;
	bool disableOutputCapture{ false };
private:
	// execve に渡す文字列の実体。_environment はこの要素を指すので、
	// 文字列の寿命を ProcessInfo と同じにしておく必要がある
	std::vector<std::string> _environmentStrings;
	// execve 用の NULL 終端ポインタ配列
	std::vector<char *> _environment;
public:
	ProcessInfo() = default;
	ProcessInfo(std::string commandLine, bool disableOutputCapture = false) : commandLine(commandLine), disableOutputCapture(disableOutputCapture) {}
	void SetEnvironment(std::vector<std::string> environment) {
		_environmentStrings = std::move(environment);
		_environment.clear();
		_environment.reserve(_environmentStrings.size() + 1);
		for (auto &e : _environmentStrings) {
			_environment.push_back(e.data());
		}
		_environment.push_back(nullptr);	// execve は NULL 終端を要求する
	}
	std::vector<char *> &GetEnvironment() {
		return _environment;
	}
};

class ProcessClass : public Process {
public:
	ProcessClass() {
		Process_Init(this, nullptr);
	}
	ProcessClass(ProcessInfo &info) {
		ProcessStartInfo startInfo;
		startInfo.commandLine = info.commandLine.c_str();
		startInfo.environment = info.GetEnvironment().empty()
			? nullptr : info.GetEnvironment().data();
		startInfo.disableOutputCapture = info.disableOutputCapture;
		Process_Init(this, &startInfo);
	}
	~ProcessClass() {
		Process_Destroy(this);
	}

	bool HasExited() {
		return Process_HasExited(this);
	}

	int32_t ExitCode() {
		return Process_ExitCode(this);
	}
	double TotalProcessorTime() {
		return Process_TotalProcessorTime(this);
	}
	double UserProcessorTime() {
		return Process_UserProcessorTime(this);
	}
	double StartTime() {
		return Process_StartTime(this);
	}
	double ExitTime() {
		return Process_ExitTime(this);
	}
	pid_t Id() {
		return Process_Id(this);
	}
	std::string StandardOutput() {
		ProcessOutput output;
		Process_StandardOutput(this, &output);
		if (output.size == 0) {
			ProcessOutput_Destroy(&output);
			return "";
		}
		char *buffer = ProcessOutput_Read(&output);
		ProcessOutput_Destroy(&output);
		std::string text{buffer};
		free(buffer);
		return text;
	}
	std::string StandardError() {
		ProcessOutput output;
		Process_StandardError(this, &output);
		if (output.size == 0) {
			ProcessOutput_Destroy(&output);
			return "";
		}
		std::vector<char> buffer(output.size + 1);
		buffer[output.size] = '\0';
		ProcessOutput_ReadTo(&output, buffer.data(), output.size + 1);
		ProcessOutput_Destroy(&output);
		return std::string(buffer.data());
	}

	void Start() {
		Process_Start(this);
	}

	void Wait() {
		Process_Wait(this);
	}

	bool WaitFor(int32_t milliseconds) {
		return Process_WaitFor(this, milliseconds);
	}

	void PrintSummary() {
		Process_PrintSummary(this);
	}

	void SetStartInfo(ProcessInfo &info) {
		this->info.commandLine = info.commandLine.c_str();
		if (!info.GetEnvironment().empty()) {
			this->info.environment = info.GetEnvironment().data();
		}
		this->info.disableOutputCapture = info.disableOutputCapture;
	}
	static Process Execute(ProcessInfo &info) {
		ProcessStartInfo startInfo;
		startInfo.commandLine = info.commandLine.c_str();
		if (!info.GetEnvironment().empty()) {
			startInfo.environment = info.GetEnvironment().data();
		}
		startInfo.disableOutputCapture = info.disableOutputCapture;
		ProcessClass instance;
		Process_Execute(&instance, &startInfo);
		return instance;
	}
};
