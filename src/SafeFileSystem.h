
#pragma once

#include "ProcessUtility.h"
#include "spdlog/spdlog.h"
#include <filesystem>
#include <functional>

namespace fs = std::filesystem;

class SafeFileSystem
{
  public:
	static inline uintmax_t fileSizeThread(const fs::path &path, int timeoutSeconds = 5, std::string referenceToLog = "")
	{
		return withTimeoutThread<std::function<uintmax_t()>, uintmax_t>([path]() { return fs::file_size(path); }, timeoutSeconds, referenceToLog);
	}
	static inline uintmax_t fileSizeProcess(const fs::path &path, int timeoutSeconds = 5, std::string referenceToLog = "")
	{
		int maxRetries = 2;
		int currentRetries = 0;
		int exitStatus;
		do
		{
			SPDLOG_INFO(
				"forkAndExec. fileSizeProcess"
				"{}"
				", path: {}"
				", timeoutSeconds: {}"
				", currentRetries: {}/{}",
				referenceToLog, path.string(), timeoutSeconds, currentRetries++, maxRetries
			);
			exitStatus = ProcessUtility::forkAndExec<std::function<uintmax_t()>>([path]() { return fs::file_size(path); }, timeoutSeconds, referenceToLog);
		} while (exitStatus == -3 && currentRetries < maxRetries); // timeout

		if (exitStatus < 0)
		{
			std::string errorMessage = std::format(
				"forkAndExec. fileSizeProcess failed"
				"{}"
				", path: {}"
				", timeoutSeconds: {}"
				", currentRetries: {}/{}",
				referenceToLog, path.string(), timeoutSeconds, currentRetries, maxRetries
			);
			SPDLOG_ERROR(errorMessage);

			throw std::runtime_error(errorMessage);
		}

		return exitStatus;
	}

	static inline bool existsThread(const fs::path &path, int timeoutSeconds = 5, std::string referenceToLog = "")
	{
		return withTimeoutThread<std::function<bool()>, bool>([path]() { return fs::exists(path); }, timeoutSeconds, referenceToLog);
	}
	static inline bool existsProcess(const fs::path &path, int timeoutSeconds = 5, std::string referenceToLog = "")
	{
		return ProcessUtility::forkAndExec<std::function<bool()>>([path]() { return fs::exists(path); }, timeoutSeconds, referenceToLog);
	}

	static inline void copyThread(
		const fs::path &src, const fs::path &dst, fs::copy_options options = fs::copy_options::recursive, int timeoutSeconds = 10,
		std::string referenceToLog = ""
	)
	{
		withTimeoutThread<std::function<void()>, void>([src, dst, options]() { fs::copy(src, dst, options); }, timeoutSeconds, referenceToLog);
	}
	static inline void copyProcess(
		const fs::path &src, const fs::path &dst, fs::copy_options options = fs::copy_options::recursive, int timeoutSeconds = 10,
		std::string referenceToLog = ""
	)
	{
		ProcessUtility::forkAndExec<std::function<int()>>(
			[src, dst, options]()
			{
				fs::copy(src, dst, options);
				return 0;
			},
			timeoutSeconds, referenceToLog
		);
	}

  private:
	template <typename Func, typename ReturnType> static ReturnType withTimeoutThread(Func func, int timeoutSeconds, std::string referenceToLog);
};
