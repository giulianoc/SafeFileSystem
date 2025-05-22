
#include "SafeFileSystem.h"
#include "spdlog/spdlog.h"
#include <future>

// Per evitare blocchi/hangs su mount instabili (es. CIFS)
template <typename Func, typename ReturnType> ReturnType SafeFileSystem::withTimeoutThread(Func func, int timeoutSeconds, string referenceToLog)
{
	try
	{
		auto future = async(launch::async, func);
		if (future.wait_for(chrono::seconds(timeoutSeconds)) == std::future_status::ready)
		{
			return future.get();
		}
		else
		{
			string errorMessage = std::format(
				"SafeFileSystem timeout, thread remaining hanged!!!"
				"{}",
				referenceToLog
			);
			SPDLOG_ERROR(errorMessage);

			throw runtime_error(errorMessage);
		}
	}
	catch (const std::exception &e)
	{
		SPDLOG_ERROR(
			"filesystem exception"
			"{}"
			", e.what: {}",
			referenceToLog, e.what()
		);
		throw;
	}
}
