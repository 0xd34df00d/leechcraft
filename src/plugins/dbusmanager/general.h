/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <variant>
#include <QObject>
#include <QStringList>
#include "common.h"

namespace LC
{
namespace DBusManager
{
	class General : public QObject
	{
		Q_OBJECT
	public:
		using QObject::QObject;

		QStringList GetLoadedPlugins ();

		using Description_t = Util::Either<std::variant<IdentifierNotFound>, QString>;
		Description_t GetDescription (const QString&);

		using Icon_t = Util::Either<std::variant<IdentifierNotFound, SerializationError>, QByteArray>;
		Icon_t GetIcon (const QString&, int);
	};
}
}
