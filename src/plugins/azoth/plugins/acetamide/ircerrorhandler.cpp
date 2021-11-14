/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/


#include "ircerrorhandler.h"
#include <QTextCodec>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ientitymanager.h>
#include <util/xpc/util.h>
#include <util/xpc/notificationactionhandler.h>
#include "ircserverhandler.h"
#include "localtypes.h"

namespace LC::Azoth::Acetamide
{
	IrcErrorHandler::IrcErrorHandler (QObject *parent)
	: QObject { parent }
	{
	}

	void IrcErrorHandler::HandleError (const IrcMessageOptions& options)
	{
		if (!IsError (options.Command_.toInt ()))
			return;

		QString paramsMessage;

		if (options.Parameters_.count () > 1)
			for (const auto& str : options.Parameters_.mid (1))
				paramsMessage += QString::fromStdString (str) + " ";

		Entity e = Util::MakeNotification (Lits::AzothAcetamide,
				paramsMessage.isEmpty () ?
						options.Message_ :
						(paramsMessage + ": " + options.Message_),
				Priority::Warning);
		GetProxyHolder ()->GetEntityManager ()->HandleEntity (e);
	}

	namespace
	{
		template<int B, int E>
		struct Range
		{
			static_assert (B <= E, "Invalid range");

			bool operator() (int id) const
			{
				return B <= id && id <= E;
			}
		};

		template<int... Pts>
		struct Points
		{
			bool operator() (int id) const
			{
				return ((id == Pts) || ...);
			}
		};

		template<typename... Args>
		bool Check (int id)
		{
			return (Args {} (id) || ...);
		}
	}

	bool IrcErrorHandler::IsError (int id)
	{
		return Check<
				Range<401, 409>,
				Range<411, 415>,
				Points<421, 422, 424>,
				Range<431, 433>,
				Range<436, 437>,
				Range<441, 446>,
				Points<451>,
				Range<461, 467>,
				Range<471, 478>,
				Range<481, 485>,
				Points<491, 501, 502>
			> (id);
	}
}
