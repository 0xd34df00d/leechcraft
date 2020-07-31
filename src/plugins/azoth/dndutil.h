/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QString>

template<typename T>
class QList;

class QMimeData;
class QObject;

namespace LC
{
namespace Azoth
{
class ICLEntry;

namespace DndUtil
{
	QString GetFormatId ();

	struct MimeContactInfo
	{
		ICLEntry *Entry_;
		QString Group_;
	};

	QByteArray Encode (const QList<MimeContactInfo>&, QMimeData*);
	QObject* DecodeEntryObj (const QMimeData*);
	QList<QObject*> DecodeEntryObjs (const QMimeData*);
	QList<MimeContactInfo> DecodeMimeInfos (const QMimeData*);

	bool HasContacts (const QMimeData*);
}
}
}
