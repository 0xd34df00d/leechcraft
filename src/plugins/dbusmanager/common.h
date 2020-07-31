/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QDBusMessage>
#include <QDBusConnection>
#include <util/sll/visitor.h>
#include <util/sll/either.h>

namespace LC
{
namespace DBusManager
{
	struct IdentifierNotFound
	{
		QString Ident_;
	};

	struct SerializationError {};

	template<typename... Errs>
	QString GetErrorDescription (const std::variant<Errs...>& errs)
	{
		return Util::Visit (errs,
				[] (const IdentifierNotFound& id) { return QString { "Identifier not found: %1" }.arg (id.Ident_); },
				[] (const SerializationError&) { return QString { "Unable to serialize data" }; });
	}

	template<typename L, typename R>
	void HandleCall (Util::Either<L, R>&& result, const QDBusMessage& msg, R& output)
	{
		Util::Visit (result,
				[&output] (const R& str) { output = str; },
				[&msg] (auto errs)
				{
					const auto& descr = GetErrorDescription (errs);
					QDBusConnection::sessionBus ().send (msg.createErrorReply ("Method call failure", descr));
				});
	}
}
}
