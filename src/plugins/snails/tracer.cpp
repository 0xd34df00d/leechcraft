/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "tracer.h"
#include "accountlogger.h"

namespace LC
{
namespace Snails
{
	Tracer::Tracer (std::atomic<uint64_t>& sent, std::atomic<uint64_t>& received,
			const QString& context, int connId, const std::shared_ptr<AccountLogger>& logger)
	: Sent_ (sent)
	, Received_ (received)
	, Context_ { context }
	, ConnId_ { connId }
	, AccLogger_ { logger }
	{
	}

	void Tracer::traceReceiveBytes (const vmime::size_t count, const vmime::string& state)
	{
		Received_.fetch_add (count, std::memory_order_relaxed);

		AccLogger_->Log (Context_, ConnId_,
				QString { "received %1 bytes in state %2" }
					.arg (count)
					.arg (state.empty () ? "<null>" : state.c_str ()));
	}

	void Tracer::traceSendBytes (const vmime::size_t count, const vmime::string& state)
	{
		Sent_.fetch_add (count, std::memory_order_relaxed);

		AccLogger_->Log (Context_, ConnId_,
				QString { "sent %1 bytes in state %2" }
					.arg (count)
					.arg (state.empty () ? "<null>" : state.c_str ()));
	}

	void Tracer::traceReceive (const vmime::string& line)
	{
		Received_.fetch_add (line.size (), std::memory_order_relaxed);
		AccLogger_->Log (Context_, ConnId_,
				QString { "received:\n%1" }
					.arg (line.c_str ()));
	}

	void Tracer::traceSend (const vmime::string& line)
	{
		Sent_.fetch_add (line.size (), std::memory_order_relaxed);
		AccLogger_->Log (Context_, ConnId_,
				QString { "sent:\n%1" }
					.arg (line.c_str ()));
	}
}
}
