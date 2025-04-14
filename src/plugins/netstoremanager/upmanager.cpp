/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "upmanager.h"
#include <QApplication>
#include <QClipboard>
#include <QStandardItemModel>
#include <QFileInfo>
#include <interfaces/structures.h>
#include <interfaces/ijobholder.h>
#include <interfaces/core/ientitymanager.h>
#include <util/util.h>
#include <util/xpc/util.h>
#include <util/sll/either.h>
#include <util/sll/visitor.h>
#include <util/threads/futures.h>
#include "interfaces/netstoremanager/istorageaccount.h"
#include "interfaces/netstoremanager/istorageplugin.h"
#include "interfaces/netstoremanager/isupportfilelistings.h"
#include "xmlsettingsmanager.h"
#include "utils.h"

inline uint qHash (const QStringList& id)
{
	return qHash (id.join ("/"));
}

namespace LC
{
namespace NetStoreManager
{
	UpManager::UpManager (ICoreProxy_ptr proxy, QObject *parent)
	: QObject (parent)
	, ReprModel_ (new QStandardItemModel (0, 3, this))
	, Proxy_ (proxy)
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
		if (items.isEmpty ())
		{
			qWarning () << Q_FUNC_INFO
					<< "empty items list for"
					<< path;
			return;
		}
		ReprModel_->removeRow (items.first ()->row ());
	}

	IStoragePlugin* UpManager::GetSenderPlugin ()
	{
		IStorageAccount *acc = qobject_cast<IStorageAccount*> (sender ());
		return qobject_cast<IStoragePlugin*> (acc->GetParentPlugin ());
	}

	void UpManager::ScheduleAutoshare (const QString& path)
	{
		Autoshare_ << path;
	}

	void UpManager::handleUploadRequest (IStorageAccount *acc, const QString& path,
			const QByteArray& id, bool byHand)
	{
		if (!Uploads_.contains (acc))
		{
			QObject *accObj = acc->GetQObject ();
			connect (accObj,
					SIGNAL (upError (QString, QString)),
					this,
					SLOT (handleError (QString, QString)));
			connect (accObj,
					SIGNAL (upStatusChanged (QString, QString)),
					this,
					SLOT (handleUpStatusChanged (QString, QString)));
			connect (accObj,
					SIGNAL (upFinished (QByteArray, QString)),
					this,
					SLOT (handleUpFinished (QByteArray, QString)));
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
					Priority::Warning);
			Proxy_->GetEntityManager ()->HandleEntity (e);
			return;
		}

		acc->Upload (path, id);

		auto plugin = qobject_cast<IStoragePlugin*> (acc->GetParentPlugin ());

		const QFileInfo fi (path);
		QList<QStandardItem*> row
		{
			new QStandardItem (tr ("Uploading %1 to %2...")
						.arg (fi.fileName ())
						.arg (plugin->GetStorageName ())),
			new QStandardItem (),
			new QStandardItem (tr ("Initializing..."))
		};

		auto progressItem = row.at (JobHolderColumn::JobProgress);
		progressItem->setData (QVariant::fromValue<JobHolderRow> (JobHolderRow::ProcessProgress),
				CustomDataRoles::RoleJobHolderRow);
		progressItem->setData (QVariant::fromValue<ProcessStateInfo> ({
					0,
					fi.size (),
					FromUserInitiated
				}),
				JobHolderRole::ProcessState);
		ReprModel_->appendRow (row);

		ReprItems_ [acc] [path] = row;

		if (byHand &&
				XmlSettingsManager::Instance ().Property ("CopyUrlOnUpload", false).toBool ())
			ScheduleAutoshare (path);
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
				Priority::Warning);
		Proxy_->GetEntityManager ()->HandleEntity (e);
	}

	void UpManager::handleUpStatusChanged (const QString& status, const QString& filepath)
	{
		IStorageAccount *acc = qobject_cast<IStorageAccount*> (sender ());
		const auto& list = ReprItems_ [acc] [filepath];
		if (list.isEmpty ())
			return;
		list [1]->setText (status);
	}

	void UpManager::handleUpFinished (const QByteArray& id, const QString& filePath)
	{
		RemovePending (filePath);
		const auto& fileName = QFileInfo { filePath }.fileName ();
		const auto& e = Util::MakeNotification ("NetStoreManager",
				tr ("File %1 was uploaded successfully")
						.arg ("<em>" + fileName + "</em>"),
				Priority::Info);
		Proxy_->GetEntityManager ()->HandleEntity (e);

		if (!Autoshare_.remove (filePath))
			return;

		auto ifl = qobject_cast<ISupportFileListings*> (sender ());
		if (!ifl)
		{
			qWarning () << Q_FUNC_INFO
					<< "account doesn't support file listings, cannot autoshare";
			return;
		}

		Util::Sequence (this, ifl->RequestUrl (id)) >>
				Utils::HandleRequestFileUrlResult (Proxy_->GetEntityManager (),
						tr ("Failed to auto-share file %1.").arg ("<em>" + fileName + "</em>"),
						[=, this] (const QUrl& url) { emit fileUploaded (filePath, url); });
	}

	void UpManager::handleUpProgress (quint64 done, quint64 total, const QString& filepath)
	{
		IStorageAccount *acc = qobject_cast<IStorageAccount*> (sender ());
		const auto& list = ReprItems_ [acc] [filepath];
		if (list.isEmpty ())
			return;

		auto item = list.at (2);
		item->setText (tr ("%1 of %2")
				.arg (Util::MakePrettySize (done))
				.arg (Util::MakePrettySize (total)));
		Util::SetJobHolderProgress (item, done, total);
	}
}
}
