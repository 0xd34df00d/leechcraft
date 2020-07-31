/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QMap>
#include <QUrl>
#include <QString>
#include <QVariant>
#include "interfaces/poshuku/poshukutypes.h"

class QDebug;

namespace LC
{
namespace Poshuku
{
	QVariantMap ToVariantMap (const ElementData&);

	QDataStream& operator<< (QDataStream&, const ElementData&);
	QDataStream& operator>> (QDataStream&, ElementData&);

	QDebug& operator<< (QDebug&, const ElementData&);

	struct ElemFinder
	{
		const QString& ElemName_;
		const QString& ElemType_;

		ElemFinder (const QString& en, const QString& et)
		: ElemName_ (en)
		, ElemType_ (et)
		{
		}

		inline bool operator() (const ElementData& ed) const
		{
			return ed.Name_ == ElemName_ &&
					ed.Type_ == ElemType_;
		}
	};
}
}
