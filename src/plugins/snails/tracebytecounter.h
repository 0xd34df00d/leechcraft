/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <atomic>

namespace LC
{
namespace Snails
{
	class TraceByteCounter
	{
		const uint64_t InitialSent_;
		const uint64_t InitialReceived_;

		const std::atomic<uint64_t>& Sent_;
		const std::atomic<uint64_t>& Received_;
	public:
		TraceByteCounter (const std::atomic<uint64_t>&, const std::atomic<uint64_t>&);

		uint64_t GetSent () const;
		uint64_t GetReceived () const;
	};
}
}
