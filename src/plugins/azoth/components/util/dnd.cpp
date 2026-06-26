/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "dnd.h"
#include <QString>
#include <QMimeData>
#include <util/sll/qtutil.h>
#include "util/azoth/util.h"
#include "interfaces/azoth/iclentry.h"
#include "../../core.h"

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
			stream << info.Entry_->GetGlobalStrongestID () << info.Group_;

		if (data)
			data->setData (CLEntryFormat, encoded);

		return encoded;
	}

	ICLEntry* DecodeEntryObj (const QMimeData *mime)
	{
		QDataStream stream (mime->data (CLEntryFormat));
		GlobalStrongestId sid;
		stream >> sid;

		return Core::Instance ().GetEntryOrNull (sid);
	}

	QList<ICLEntry*> DecodeEntryObjs (const QMimeData *mime)
	{
		QList<ICLEntry*> result;
		for (const auto& info : DecodeMimeInfos (mime))
			result << info.Entry_;
		return result;
	}

	QList<MimeContactInfo> DecodeMimeInfos (const QMimeData *mime)
	{
		QList<MimeContactInfo> result;

		QDataStream stream (mime->data (CLEntryFormat));
		while (!stream.atEnd ())
		{
			GlobalStrongestId id;
			QString group;
			stream >> id >> group;

			if (const auto entry = Core::Instance ().GetEntryOrNull (id))
				result.append ({ entry, group });
		}

		return result;
	}

	bool HasContacts (const QMimeData *data)
	{
		return data->hasFormat (CLEntryFormat);
	}
}
