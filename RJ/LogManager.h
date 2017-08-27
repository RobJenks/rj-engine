#pragma once

#ifndef __LogManagerH__
#define __LogManagerH__

#include <fstream>
#include "CompilerSettings.h"
#include "GlobalFlags.h"
#include "ScheduledObject.h"
#include "GameVarsExtern.h"
#include "Utility.h"

// Common body of a log prefix
#define LOG_PREFIX_BODY "[" << (unsigned int)timeGetTime() << "|" << __FILE__ << ":" << __LINE__ << "] "

// Standard line prefix for a logged event
#define LOG_INFO "INFO: " << LOG_PREFIX_BODY
#define LOG_WARN "WARNING: " << LOG_PREFIX_BODY
#define LOG_ERROR "ERROR: " << LOG_PREFIX_BODY
#define LOG_DEBUG "DEBUG: " << LOG_PREFIX_BODY

// This class has no special alignment requirements
class LogManager : public ScheduledObject
{
public:
	// Enumeration of possible logging levels
	//enum Level { Error, Warning, Info, _COUNT };

	// Default constructor
	LogManager(void);


	// Operator to allow streaming directly to this LogManager object; outputs on the primary (m_stream) log
	template <typename T>
	CMPINLINE LogManager & operator<<(const T& data) 
	{
		m_stream << data;
		if (m_alwaysflush) m_stream.flush();
		return (*this);
	}

	// Function that takes a custom stream and returns it
	typedef LogManager& (*StreamManipulator)(LogManager&);

	// Stream method that accepts a function and executes it during the stream processing
	LogManager& operator<<(StreamManipulator manip)
	{
		// Call the function and return its value
		return manip(*this);
	}

	// Custom stream operators for specific types
	CMPINLINE LogManager & operator<<(const FXMVECTOR data) { return operator<<(Vector4ToString(data)); }
	CMPINLINE LogManager & operator<<(const FXMMATRIX data) { return operator<<(MatrixToString(data)); }

	// Custom function to force a flush of log data during streaming
	static LogManager& flush(LogManager& stream)
	{
		stream.LogStream().flush();
		return stream;
	}

	// Inline methods to allow access to the primary log (m_stream is also the default for streaming directly to this object)
	CMPINLINE std::ofstream &		LogStream(void)					{ return m_stream; }

	// Inline method to expose the profiling stream, if applicable
	#ifdef RJ_PROFILER_ACTIVE
		CMPINLINE std::ofstream &	ProfilingStream(void)			{ return m_profilingstream; }
	#endif

	// Enable or disable flushing of the main log data after every stream operation
	CMPINLINE void				EnableFlushAfterEveryOperation(void)	{ m_alwaysflush = true; }
	CMPINLINE void				DisableFlushAfterEveryOperation(void)	{ m_alwaysflush = false; }

	// Inherited frequent update method; no action to be taken
	CMPINLINE void Update(void) { }

	// Inherited infrequent update method; ensures any pending data is periodically flushed to the log file
	void UpdateInfrequent(void);

	// Returns a value indicating whether the primary log stream is working 
	CMPINLINE bool LoggingActive(void) { return StreamIsActive(m_stream); }

	// Directly flushes all streams, to ensure all buffered data has been written out to disk
	void FlushAllStreams(void);

	// Attempts to flush and close all logs.  Will also happen automatically upon normal destruction
	void ShutdownLogging(void);

	// Default destructor
	~LogManager(void);

	// Streaming operators for custom types
	CMPINLINE LogManager & operator<<(const INTVECTOR2 & rhs) {
		m_stream << "(" << rhs.x << ", " << rhs.y << ")";
		if (m_alwaysflush) m_stream.flush();
		return (*this); 
	}

	// Streaming operator to handle the std::endl manipulator function
	CMPINLINE friend LogManager & operator<<(LogManager & ostr, std::ostream & (*manip)(std::ostream &)) {
		manip(ostr.LogStream());
		return (ostr);
	}

protected:

	// Checks whether a particular stream is active
	CMPINLINE bool				StreamIsActive(std::ofstream & stream) { return (stream && stream.is_open() && stream.good() && !stream.fail()); }

	// Output file stream for the primary application log
	std::ofstream				m_stream;

	// Output file stream for the profiling log (only relevant when profiling is enabled)
	#ifdef RJ_PROFILER_ACTIVE
		std::ofstream			m_profilingstream;
	#endif
	
	// Flag indicating whether the primary log should flush after every stream operation.  Used during initialisation
	// to make sure that all data is output before any potential crash
	bool						m_alwaysflush;
	
};







#endif