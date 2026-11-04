/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "dnd.h"
#include <QDataStream>
#include <QIODevice>
#include <QMimeData>
#include <QString>
#include <util/sll/qtutil.h>
#include "util/azoth/entryidops.h"
#include "interfaces/azoth/iclentry.h"
#include "components/managers/entriesmanager.h"

namespace LC::Azoth::DndUtil
{
	const QString CLEntryFormat = "x-leechcraft/azoth-cl-entry"_qs;

	QString GetFormatId ()
	{
		return CLEntryFormat;
	}

	QByteArray Encode (const QList<MimeContactInfo>& entries, QMimeData *data)
	{
		QByteArray encoded;
		QDataStream stream (&encoded, QIODevice::WriteOnly);
		for (const auto& info : entries)
		{
			Serialize (stream, info.Entry_->GetGlobalStrongestID ());
			stream << info.Group_;
		}

		if (data)
			data->setData (CLEntryFormat, encoded);

		return encoded;
	}

	ICLEntry* DecodeEntryObj (const EntriesManager& entries, const QMimeData *mime)
	{
		QDataStream stream (mime->data (CLEntryFormat));
		const auto id = Deserialize<GlobalStrongestId> (stream);
		return entries.GetEntryOrNull (id);
	}

	QList<ICLEntry*> DecodeEntryObjs (const EntriesManager& entries, const QMimeData *mime)
	{
		QList<ICLEntry*> result;
		for (const auto& info : DecodeMimeInfos (entries, mime))
			result << info.Entry_;
		return result;
	}

	QList<MimeContactInfo> DecodeMimeInfos (const EntriesManager& entries, const QMimeData *mime)
	{
		QList<MimeContactInfo> result;

		QDataStream stream (mime->data (CLEntryFormat));
		while (!stream.atEnd ())
		{
			const auto id = Deserialize<GlobalStrongestId> (stream);
			if (!id)
				break;

			QString group;
			stream >> group;
			if (const auto entry = entries.GetEntryOrNull (id))
				result.append ({ entry, group });
		}

		return result;
	}

	bool HasContacts (const QMimeData *data)
	{
		return data->hasFormat (CLEntryFormat);
	}
}
