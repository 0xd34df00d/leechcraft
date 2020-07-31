/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QStringList>
#include <QVariantList>
#include "common.h"

namespace LC
{
namespace DBusManager
{
	class Tasks : public QObject
	{
		Q_OBJECT
	public:
		using QObject::QObject;

		QStringList GetHolders () const;

		using RowCountResult_t = Util::Either<std::variant<IdentifierNotFound>, int>;
		RowCountResult_t RowCount (const QString& holder) const;

		using GetDataResult_t = Util::Either<std::variant<IdentifierNotFound>, QVariantList>;
		GetDataResult_t GetData (const QString&, int, int) const;
	};
}
}
