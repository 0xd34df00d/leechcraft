/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <atomic>
#include <QString>
#include <vmime/net/tracer.hpp>

namespace LC
{
namespace Snails
{
	class AccountLogger;

	class TraceByteCounter;

	class TracerFactory : public vmime::net::tracerFactory
	{
		const std::shared_ptr<AccountLogger> AccLogger_;
		const QString Context_;

		std::atomic<uint64_t> SentBytes_ { 0 };
		std::atomic<uint64_t> ReceivedBytes_ { 0 };
	public:
		TracerFactory (const QString&, const std::shared_ptr<AccountLogger>&);

		TraceByteCounter CreateCounter ();

		vmime::shared_ptr<vmime::net::tracer> create (const vmime::shared_ptr<vmime::net::service>&, const int) override;
	};
}
}
