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

	class Tracer : public vmime::net::tracer
	{
		std::atomic<uint64_t>& Sent_;
		std::atomic<uint64_t>& Received_;

		const QString Context_;
		const int ConnId_;
		const std::shared_ptr<AccountLogger> AccLogger_;
	public:
		Tracer (std::atomic<uint64_t>&, std::atomic<uint64_t>&,
		        const QString&, int, const std::shared_ptr<AccountLogger>&);

		void traceReceiveBytes (const vmime::size_t count, const vmime::string& state) override;
		void traceSendBytes (const vmime::size_t count, const vmime::string& state) override;
		void traceReceive (const vmime::string& line) override;
		void traceSend (const vmime::string& line) override;
	};
}
}
