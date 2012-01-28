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
#include <QStandardItemModel>
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
	, ReprModel_ (new QStandardItemModel (0, 3, this))
	{
	}

	QAbstractItemModel* UpManager::GetRepresentationModel () const
	{
		return ReprModel_;
	}

	void UpManager::RemovePending (const QString& path)
	{
		IStorageAccount *acc = qobject_cast<IStorageAccount*> (sender ());
		Uploads_ [acc].removeAll (path);

		auto items = ReprItems_ [acc].take (path);
		ReprModel_->removeRow (items.first ()->row ());
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
			connect (accObj,
					SIGNAL (upStatusChanged (QString, QString)),
					this,
					SLOT (handleUpStatusChanged (QString, QString)));
			connect (accObj,
					SIGNAL (upProgress (quint64, quint64, QString)),
					this,
					SLOT (handleUpProgress (quint64, quint64, QString)));
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

		auto plugin = qobject_cast<IStoragePlugin*> (acc->GetParentPlugin ());

		QList<QStandardItem*> row;
		row << new QStandardItem (tr ("Uploading %1 to %2...")
					.arg (QFileInfo (path).fileName ())
					.arg (plugin->GetStorageName ()));
		row << new QStandardItem ();
		row << new QStandardItem (tr ("Initializing..."));
		ReprModel_->appendRow (row);

		ReprItems_ [acc] [path] = row;
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
		emit gotEntity (e);
	}

	void UpManager::handleUpStatusChanged (const QString& status, const QString& filepath)
	{
		IStorageAccount *acc = qobject_cast<IStorageAccount*> (sender ());
		const auto& list = ReprItems_ [acc] [filepath];
		if (list.isEmpty ())
			return;
		list [2]->setText (status);
	}

	void UpManager::handleUpProgress (quint64 done, quint64 total, const QString& filepath)
	{
		IStorageAccount *acc = qobject_cast<IStorageAccount*> (sender ());
		const auto& list = ReprItems_ [acc] [filepath];
		if (list.isEmpty ())
			return;
		list [1]->setText (tr ("%1 of %2")
				.arg (Util::MakePrettySize (done))
				.arg (Util::MakePrettySize (total)));
	}
}
}
