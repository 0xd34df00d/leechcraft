/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "pageformsdata.h"
#include <QDataStream>
#include <QtDebug>

namespace LC
{
namespace Poshuku
{
	QVariantMap ToVariantMap (const ElementData& ed)
	{
		return
		{
			{ "PageURL", ed.PageURL_ },
			{ "FormID", ed.FormID_ },
			{ "Name", ed.Name_ },
			{ "Type", ed.Type_ },
			{ "Value", ed.Value_ }
		};
	}

	QDataStream& operator<< (QDataStream& out, const ElementData& ed)
	{
		out << static_cast<quint8> (1)
			<< ed.PageURL_
			<< ed.FormID_
			<< ed.Name_
			<< ed.Type_
			<< ed.Value_;
		return out;
	}

	QDataStream& operator>> (QDataStream& in, ElementData& ed)
	{
		quint8 version = 0;
		in >> version;
		if (version == 1)
			in >> ed.PageURL_
				>> ed.FormID_
				>> ed.Name_
				>> ed.Type_
				>> ed.Value_;
		else
			qWarning () << Q_FUNC_INFO
				<< "unable to deserialize ElementType of version"
				<< version;

		return in;
	}

	QDebug& operator<< (QDebug& dbg, const ElementData& ed)
	{
		dbg << "Element: {"
			<< ed.PageURL_
			<< ed.FormID_
			<< ed.Name_
			<< ed.Type_
			<< ed.Value_
			<< "}";
		return dbg;
	}
}
}
