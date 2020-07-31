/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "tracebytecounter.h"

namespace LC
{
namespace Snails
{
	TraceByteCounter::TraceByteCounter (const std::atomic<uint64_t>& sent, const std::atomic<uint64_t>& received)
	: InitialSent_ { sent.load (std::memory_order_relaxed) }
	, InitialReceived_ { received.load (std::memory_order_relaxed) }
	, Sent_ (sent)
	, Received_ (received)
	{
	}

	uint64_t TraceByteCounter::GetSent () const
	{
		return Sent_.load (std::memory_order_relaxed) - InitialSent_;
	}

	uint64_t TraceByteCounter::GetReceived () const
	{
		return Received_.load (std::memory_order_relaxed) - InitialReceived_;
	}
}
}
