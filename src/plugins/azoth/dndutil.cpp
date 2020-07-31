/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "dndutil.h"
#include <QString>
#include <QList>
#include <QDataStream>
#include <QMimeData>
#include "interfaces/azoth/iclentry.h"
#include "core.h"

namespace LC
{
namespace Azoth
{
namespace DndUtil
{
	const QString CLEntryFormat = "x-leechcraft/azoth-cl-entry";

	QString GetFormatId ()
	{
		return CLEntryFormat;
	}

	QByteArray Encode (const QList<MimeContactInfo>& entries, QMimeData *data)
	{
		QByteArray encoded;
		QDataStream stream (&encoded, QIODevice::WriteOnly);
		for (const auto& info : entries)
			stream << info.Entry_->GetEntryID () << info.Group_;

		if (data)
			data->setData (CLEntryFormat, encoded);

		return encoded;
	}

	QObject* DecodeEntryObj (const QMimeData *mime)
	{
		QDataStream stream (mime->data (CLEntryFormat));
		QString sid;
		stream >> sid;

		return Core::Instance ().GetEntry (sid);
	}

	QList<QObject*> DecodeEntryObjs (const QMimeData *mime)
	{
		QList<QObject*> result;
		for (const auto& info : DecodeMimeInfos (mime))
			result << info.Entry_->GetQObject ();
		return result;
	}

	QList<MimeContactInfo> DecodeMimeInfos (const QMimeData *mime)
	{
		QList<MimeContactInfo> result;

		QDataStream stream (mime->data (CLEntryFormat));
		while (!stream.atEnd ())
		{
			QString id;
			QString group;
			stream >> id >> group;

			const auto entryObj = Core::Instance ().GetEntry (id);
			const auto entry = qobject_cast<ICLEntry*> (entryObj);
			if (!entry)
				continue;

			result.append ({ entry, group });
		}

		return result;
	}

	bool HasContacts (const QMimeData *data)
	{
		return data->hasFormat (CLEntryFormat);
	}
}
}
}
