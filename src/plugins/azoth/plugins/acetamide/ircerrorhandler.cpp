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

namespace LC
{
namespace Azoth
{
namespace Acetamide
{
	IrcErrorHandler::IrcErrorHandler (QObject *parent)
	: QObject (parent)
	{
		InitErrors ();
	}

	void IrcErrorHandler::HandleError (const IrcMessageOptions& options)
	{
		if (!IsError (options.Command_.toInt ()))
			return;

		QString paramsMessage;

		if (options.Parameters_.count () > 1)
			for (const auto& str : options.Parameters_.mid (1))
				paramsMessage += QString::fromUtf8 (str.c_str ()) + " ";

		Entity e = Util::MakeNotification ("Azoth",
				paramsMessage.isEmpty () ?
						options.Message_ :
						(paramsMessage + ": " + options.Message_),
				Priority::Warning);
		GetProxyHolder ()->GetEntityManager ()->HandleEntity (e);
	}

	bool IrcErrorHandler::IsError (int id)
	{
		return ErrorKeys_.contains (id);
	}

	namespace
	{
		template<int B, int E>
		struct Range
		{
			static_assert (B <= E, "Invalid range");

			constexpr static int Begin = B;
			constexpr static int End = E;
		};

		template<int... Pts>
		struct Points
		{
		};

		template<typename T, typename... Args>
		struct Filler;

		template<typename T, int... Pts, typename... Args>
		struct Filler<T, Points<Pts...>, Args...>
		{
			void operator() (T& list) const
			{
				list += T { Pts... };

				Filler<T, Args...> {} (list);
			}
		};

		template<typename T, typename Arg, typename... Args>
		struct Filler<T, Arg, Args...>
		{
			void operator() (T& list) const
			{
				for (int i = Arg::Begin; i <= Arg::End; ++i)
					list << i;

				Filler<T, Args...> {} (list);
			}
		};

		template<typename T>
		struct Filler<T>
		{
			void operator() (T&) const
			{
			}
		};
	}

	void IrcErrorHandler::InitErrors ()
	{
		Filler<
				decltype (ErrorKeys_),
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
			> {} (ErrorKeys_);
	}
}
}
}
