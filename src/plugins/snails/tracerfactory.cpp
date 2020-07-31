/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "tracerfactory.h"
#include "tracer.h"
#include "tracebytecounter.h"

namespace LC
{
namespace Snails
{
	TracerFactory::TracerFactory (const QString& context, const std::shared_ptr<AccountLogger>& logger)
	: AccLogger_ { logger }
	, Context_ { context }
	{
	}

	TraceByteCounter TracerFactory::CreateCounter ()
	{
		return { SentBytes_, ReceivedBytes_ };
	}

	vmime::shared_ptr<vmime::net::tracer> TracerFactory::create (const vmime::shared_ptr<vmime::net::service>&, const int connId)
	{
		return vmime::make_shared<Tracer> (SentBytes_, ReceivedBytes_, Context_, connId, AccLogger_);
	}
}
}
