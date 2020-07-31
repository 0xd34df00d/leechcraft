/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QString>

class QUrl;

namespace LC
{
namespace Azoth
{
	class ICLEntry;

	class PendingUploadPaster : public QObject
	{
		Q_OBJECT

		ICLEntry *Entry_;
		const QString EntryVariant_;
		const QString Filename_;
	public:
		PendingUploadPaster (QObject *sharer, ICLEntry *entry, const QString& variant, const QString& filename, QObject* = 0);
	private slots:
		void handleFileUploaded (const QString&, const QUrl&);
	};
}
}
