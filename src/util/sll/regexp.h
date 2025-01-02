/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include "sllconfig.h"
#include <memory>
#include <QString>
#include <QMetaType>
#include <QRegularExpression>

namespace LC
{
namespace Util
{
	// TODO mark this deprecated once Qt6 port is complete
	class UTIL_SLL_API RegExp
	{
		QRegularExpression Rx_;
	public:
		static bool IsFast ();

		RegExp () = default;
		RegExp (const QString&, Qt::CaseSensitivity);

		bool Matches (const QString&) const;
		bool Matches (const QByteArray&) const;

		QString GetPattern () const;
		Qt::CaseSensitivity GetCaseSensitivity () const;
	};
}
}

UTIL_SLL_API QDataStream& operator<< (QDataStream&, const LC::Util::RegExp&);
UTIL_SLL_API QDataStream& operator>> (QDataStream&, LC::Util::RegExp&);

Q_DECLARE_METATYPE (LC::Util::RegExp)
