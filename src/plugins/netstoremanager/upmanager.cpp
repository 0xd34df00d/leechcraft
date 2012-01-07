/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "upmanager.h"
#include <QApplication>
#include <QClipboard>
#include <interfaces/structures.h>
#include <util/util.h>
#include "interfaces/netstoremanager/istorageaccount.h"
#include "interfaces/netstoremanager/istorageplugin.h"

namespace LeechCraft
{
namespace NetStoreManager
{
	UpManager::UpManager (QObject *parent)
	: QObject (parent)
	{
	}

	void UpManager::RemovePending (const QString& path)
	{
		IStorageAccount *acc = qobject_cast<IStorageAccount*> (sender ());
		Uploads_ [acc].removeAll (path);
	}

	IStoragePlugin* UpManager::GetSenderPlugin ()
	{
		IStorageAccount *acc = qobject_cast<IStorageAccount*> (sender ());
		return qobject_cast<IStoragePlugin*> (acc->GetParentPlugin ());
	}

	void UpManager::handleUploadRequest (IStorageAccount *acc, const QString& path)
	{
		if (!Uploads_.contains (acc))
		{
			QObject *accObj = acc->GetObject ();
			connect (accObj,
					SIGNAL (gotURL (QUrl, QString)),
					this,
					SLOT (handleGotURL (QUrl, QString)));
			connect (accObj,
					SIGNAL (upError (QString, QString)),
					this,
					SLOT (handleError (QString, QString)));
		}
		else if (Uploads_ [acc].contains (path))
		{
			const Entity& e = Util::MakeNotification ("NetStoreManager",
					tr ("%1 is already uploading to %2.")
						.arg (QFileInfo (path).fileName ())
						.arg (acc->GetAccountName ()),
					PWarning_);
			emit gotEntity (e);
			return;
		}

		acc->Upload (path);
	}

	void UpManager::handleGotURL (const QUrl& url, const QString& path)
	{
		const QString& urlStr = url.toString ();
		qApp->clipboard ()->setText (urlStr, QClipboard::Clipboard);
		qApp->clipboard ()->setText (urlStr, QClipboard::Selection);

		RemovePending (path);

		auto plugin = GetSenderPlugin ();
		const Entity& e = Util::MakeNotification (plugin->GetStorageName (),
				tr ("%1 is successfully uploaded, URL is pasted into clipboard.")
					.arg (QFileInfo (path).fileName ()),
				PInfo_);
		emit gotEntity (e);
	}

	void UpManager::handleError (const QString& str, const QString& path)
	{
		qWarning () << Q_FUNC_INFO << str << path;

		RemovePending (path);

		auto plugin = GetSenderPlugin ();
		const Entity& e = Util::MakeNotification (plugin->GetStorageName (),
				tr ("Failed to upload %1: %2.")
					.arg (path)
					.arg (str),
				PWarning_);
	}
}
}
