/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "pendinguploadpaster.h"
#include <QUrl>
#include "interfaces/azoth/iclentry.h"
#include "util.h"

namespace LC
{
namespace Azoth
{
	PendingUploadPaster::PendingUploadPaster (QObject *sharer,
			ICLEntry *entry, const QString& variant, const QString& filename, QObject *parent)
	: QObject (parent)
	, Entry_ (entry)
	, EntryVariant_ (variant)
	, Filename_ (filename)
	{
		connect (sharer,
				SIGNAL (fileUploaded (QString, QUrl)),
				this,
				SLOT (handleFileUploaded (QString, QUrl)));
	}

	void PendingUploadPaster::handleFileUploaded (const QString& filename, const QUrl& url)
	{
		if (filename != Filename_)
			return;

		SendMessage (*Entry_, { .Variant_ = EntryVariant_, .Body_ = url.toEncoded () });
		deleteLater ();
	}
}
}
