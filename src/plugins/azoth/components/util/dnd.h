/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QString>
#include <util/sll/trackingpointer.h>
#include "interfaces/azoth/iclentry.h"

template<typename T>
class QList;

class QMimeData;

namespace LC::Azoth
{
	class ChatTabsManager;
	class EntriesManager;
	class TransferJobManager;
}

namespace LC::Azoth::DndUtil
{
	QString GetFormatId ();

	struct MimeContactInfo
	{
		Util::TrackingPointer<ICLEntry> Entry_;
		QString Group_;
	};

	QByteArray Encode (const QList<MimeContactInfo>&, QMimeData*);
	ICLEntry* DecodeEntryObj (const EntriesManager&, const QMimeData*);
	QList<Util::TrackingPointer<ICLEntry>> DecodeEntryObjs (const EntriesManager&, const QMimeData*);
	QList<MimeContactInfo> DecodeMimeInfos (const EntriesManager&, const QMimeData*);

	bool HasContacts (const QMimeData*);

	struct DndContext
	{
		const EntriesManager& Entries_;
		ChatTabsManager& Tabs_;
		TransferJobManager& Transfers_;
		QWidget *Parent_;
	};

	bool HandleDroppedOnto (const QMimeData&, const Util::TrackingPointer<ICLEntry>&, const DndContext&);
}
