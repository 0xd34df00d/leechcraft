/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "webfilestorageadaptor.h"
#include <QUrl>
#include <interfaces/iwebfilestorage.h>
#include "core.h"

namespace LC
{
namespace DBusManager
{
	WebFileStorageAdaptor::WebFileStorageAdaptor (QObject *parent)
	: QDBusAbstractAdaptor (parent)
	, WFS_ (qobject_cast<IWebFileStorage*> (parent))
	{
		setAutoRelaySignals (false);
		connect (parent,
				SIGNAL (fileUploaded (QString, QUrl)),
				this,
				SLOT (handleFileUploaded (QString, QUrl)));
	}

	QStringList WebFileStorageAdaptor::GetServiceVariants () const
	{
		return WFS_->GetServiceVariants ();
	}

	void WebFileStorageAdaptor::UploadFile (const QString& filename, const QString& service)
	{
		WFS_->UploadFile (filename, service);
	}

	void WebFileStorageAdaptor::handleFileUploaded (const QString& filename, const QUrl& url)
	{
		emit FileUploaded (filename, url.toString ());
	}
}
}
