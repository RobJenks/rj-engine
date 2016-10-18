#include "LogManager.h"

// Default constructor
LogManager::LogManager(void)
{
	// Create the primary game log
	m_stream = std::ofstream("log.txt", std::ofstream::out | std::ofstream::trunc);
	
	// No file-open error handling for now, add later

	// Create the profiling log, if relevant
	#ifdef RJ_PROFILER_ACTIVE
		m_profilingstream = std::ofstream("profiling.txt", std::ofstream::out | std::ofstream::trunc);
		m_profilingstream << "Clock time(ms), Profiling group ID, Profiling group, Total clocks (ms), Iterations, Clocks per iteration (ms)\n";
		m_profilingstream.flush();
	#endif

	// By default the log will not flush after every operation
	m_alwaysflush = false;
}


// Inherited infrequent update method; ensures any pending data is periodically flushed to the log file
void LogManager::UpdateInfrequent(void)
{
	// Perform a periodic flush of the stream to reduce the chance of data being lost in the event of a crash
	FlushAllStreams();
}

// Directly flushes all streams, to ensure all buffered data has been written out to disk
void LogManager::FlushAllStreams(void)
{
	// Flush the primary log
	m_stream.flush();

	// Also flush the profiling data stream, if applicable
#	ifdef RJ_PROFILER_ACTIVE
		m_profilingstream.flush();
#	endif
}

// Attempts to flush and close all logs.  Will also happen automatically upon normal destruction
void LogManager::ShutdownLogging(void)
{
	// Attempt to shut down the main log
	if (LoggingActive())
	{
		m_stream.flush();
		m_stream.close();
	}

	// Also attempt to shut down the profiling log, if applicable
	#ifdef RJ_PROFILER_ACTIVE
		if (StreamIsActive(m_profilingstream))
		{
			m_profilingstream.flush();
			m_profilingstream.close();
		}
	#endif

}


// Default destructor
LogManager::~LogManager(void)
{
	// Close the primary game log (should also happen automatically upon destruction)
	m_stream.close();

	// Close the profiling log (If relevant.  Will also auto-close, like all streams)
	#ifdef RJ_PROFILER_ACTIVE
		m_profilingstream.close();
	#endif
}

