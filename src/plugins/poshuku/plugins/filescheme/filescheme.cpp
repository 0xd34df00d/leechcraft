/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "filescheme.h"
#include <QIcon>
#include <QFileInfo>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/iiconthememanager.h>
#include "schemereply.h"

Q_DECLARE_METATYPE (QNetworkReply*);

namespace LC
{
namespace Poshuku
{
namespace FileScheme
{
	void FileScheme::Init (ICoreProxy_ptr)
	{
	}

	void FileScheme::SecondInit ()
	{
	}

	void FileScheme::Release ()
	{
	}

	QByteArray FileScheme::GetUniqueID () const
	{
		return "org.LeechCraft.Poshuku.FileScheme";
	}

	QString FileScheme::GetName () const
	{
		return "Poshuku FileScheme";
	}

	QString FileScheme::GetInfo () const
	{
		return tr ("Provides support for file:// scheme.");
	}

	QIcon FileScheme::GetIcon () const
	{
		return GetProxyHolder ()->GetIconThemeManager ()->GetPluginIcon ();
	}

	QSet<QByteArray> FileScheme::GetPluginClasses () const
	{
		QSet<QByteArray> result;
		result << "org.LeechCraft.Poshuku.Plugins/1.0";
		result << "org.LeechCraft.Core.Plugins/1.0";
		return result;
	}

	void FileScheme::hookNAMCreateRequest (IHookProxy_ptr proxy,
			QNetworkAccessManager*,
			QNetworkAccessManager::Operation *op,
			QIODevice**)
	{
		if (*op != QNetworkAccessManager::GetOperation)
			return;

		const QNetworkRequest& req = proxy->GetValue ("request").value<QNetworkRequest> ();
		const QUrl& url = req.url ();
		if (url.scheme () != "file" ||
				!QFileInfo (url.toLocalFile ()).isDir ())
			return;

		proxy->CancelDefault ();
		proxy->SetReturnValue (QVariant::fromValue<QNetworkReply*> (new SchemeReply (req, this)));
	}
}
}
}

LC_EXPORT_PLUGIN (leechcraft_poshuku_filescheme,
		LC::Poshuku::FileScheme::FileScheme);
