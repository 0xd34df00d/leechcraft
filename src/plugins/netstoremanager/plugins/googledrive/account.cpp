/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "account.h"
#include <algorithm>
#include <QtDebug>
#include <QStandardItemModel>
#include <QMessageBox>
#include <QMainWindow>
#include <QPushButton>
#include <QFuture>
#include <util/sll/either.h>
#include <util/threads/futures.h>
#include <interfaces/core/irootwindowsmanager.h>
#include "core.h"
#include "uploadmanager.h"
#include "xmlsettingsmanager.h"

namespace LC
{
namespace NetStoreManager
{
namespace GoogleDrive
{
	Account::Account (const QString& name, QObject *parentPlugin)
	: QObject (parentPlugin)
	, ParentPlugin_ (parentPlugin)
	, Name_ (name)
	, DriveManager_ (new DriveManager (this, this))
	{
		connect (DriveManager_,
				SIGNAL (gotNewItem (DriveItem)),
				this,
				SLOT (handleGotNewItem (DriveItem)));
		connect (DriveManager_,
				SIGNAL (gotChanges (QList<DriveChanges>)),
				this,
				SLOT (handleGotChanges (QList<DriveChanges>)));
	}

	QObject* Account::GetQObject ()
	{
		return this;
	}

	QObject* Account::GetParentPlugin () const
	{
		return ParentPlugin_;
	}

	QByteArray Account::GetUniqueID () const
	{
		return "NetStoreManager.GoogleDrive_" + Name_.toUtf8 ();
	}

	AccountFeatures Account::GetAccountFeatures () const
	{
		return FileListings;
	}

	QString Account::GetAccountName () const
	{
		return Name_;
	}

	void Account::Upload (const QString& filepath, const QByteArray& parentId,
			UploadType ut, const QByteArray& id)
	{
		auto uploadManager = new UploadManager (filepath,
				ut, parentId, this, id);

		connect (uploadManager,
				SIGNAL (uploadProgress (quint64, quint64, QString)),
				this,
				SIGNAL (upProgress (quint64, quint64, QString)));
		connect (uploadManager,
				SIGNAL (uploadError (QString, QString)),
				this,
				SIGNAL (upError (QString, QString)));
		connect (uploadManager,
				SIGNAL (finished (QByteArray, QString)),
				this,
				SIGNAL (upFinished (QByteArray, QString)));
		connect (uploadManager,
				SIGNAL (uploadStatusChanged (QString, QString)),
				this,
				SIGNAL (upStatusChanged (QString, QString)));
	}

	void Account::Download (const QByteArray& id, const QString& filepath,
			TaskParameters tp, bool open)
	{
		if (id.isEmpty ())
			return;

		DriveManager_->Download (id, filepath, tp, open);
	}

	void Account::DownloadFile (const QUrl& url, const QString& filepath,
			TaskParameters tp, bool open)
	{
		emit downloadFile (url, filepath, tp, open);
	}

	ListingOps Account::GetListingOps () const
	{
		return ListingOp::Delete | ListingOp::TrashSupporting | ListingOp::DirectorySupport;
	}

	HashAlgorithm Account::GetCheckSumAlgorithm () const
	{
		return HashAlgorithm::Md5;
	}

	QFuture<Account::RefreshResult_t> Account::RefreshListing ()
	{
		return DriveManager_->RefreshListing ();
	}

	void Account::RefreshChildren (const QByteArray& parentId)
	{
		emit listingUpdated (parentId);
	}

	void Account::Delete (const QList<QByteArray>& ids, bool ask)
	{
		if (ids.isEmpty ())
			return;

		if (ask)
		{
			auto rootWM = Core::Instance ().GetProxy ()->GetRootWindowsManager ();
			auto res = QMessageBox::warning (rootWM->GetPreferredWindow (),
					"LeechCraft",
					tr ("Are you sure you want to delete all selected items? This action cannot be undone."
						"<br><i>Note: if you delete a directory then all files in it will also be deleted.</i>"),
					QMessageBox::Ok | QMessageBox::Cancel);
			if (res != QMessageBox::Ok)
				return;
		}

		for (const auto& id : ids)
			DriveManager_->RemoveEntry (id);
	}

	void Account::MoveToTrash (const QList<QByteArray>& ids)
	{
		for (const auto& id : ids)
			DriveManager_->MoveEntryToTrash (id);
	}

	void Account::RestoreFromTrash (const QList<QByteArray>& ids)
	{
		for (const auto& id : ids)
			DriveManager_->RestoreEntryFromTrash (id);
	}

	void Account::Copy (const QList<QByteArray>& ids,
			const QByteArray& newParentId)
	{
		for (const auto& id : ids)
			DriveManager_->Copy (id, newParentId);
	}

	void Account::Move (const QList<QByteArray>& ids,
			const QByteArray& newParentId)
	{
		for (const auto& id : ids)
			DriveManager_->Move (id, newParentId);
	}

	QFuture<Account::RequestUrlResult_t> Account::RequestUrl (const QByteArray& id)
	{
		if (id.isNull ())
			return Util::MakeReadyFuture (RequestUrlResult_t { InvalidItem {} });

		if (!XmlSettingsManager::Instance ().property ("AutoShareOnUrlRequest").toBool ())
		{
			auto rootWM = Core::Instance ().GetProxy ()->GetRootWindowsManager ();
			QMessageBox mbox (QMessageBox::Question,
					tr ("Share item"),
					tr ("The item needs to be shared to obtain the URL. Do you want to share it?"),
					QMessageBox::Yes | QMessageBox::No,
					rootWM->GetPreferredWindow());
			mbox.setDefaultButton (QMessageBox::Yes);

			QPushButton always (tr ("Always"));
			mbox.addButton (&always, QMessageBox::AcceptRole);

			if (mbox.exec () == QMessageBox::No)
				return Util::MakeReadyFuture (RequestUrlResult_t { UserCancelled {} });
			if (mbox.clickedButton () == &always)
				XmlSettingsManager::Instance ().setProperty ("AutoShareOnUrlRequest", true);
		}

		return DriveManager_->ShareEntry (id).then (RequestUrlResult_t::EmbeddingLeft ());
	}

	void Account::CreateDirectory (const QString& name, const QByteArray& parentId)
	{
		if (name.isEmpty ())
			return;
		DriveManager_->CreateDirectory (name, parentId);
	}

	void Account::Rename (const QByteArray& id, const QString& newName)
	{
		if (id.isEmpty ())
			return;
		DriveManager_->Rename (id, newName);
	}

	void Account::RequestChanges ()
	{
		DriveManager_->RequestFileChanges (XmlSettingsManager::Instance ()
				.Property ("LastChangesId", 0).toLongLong ());
	}

	QByteArray Account::Serialize ()
	{
		QByteArray result;
		QDataStream str (&result, QIODevice::WriteOnly);
		str << static_cast<quint8> (1)
				<< Name_
				<< Trusted_
				<< RefreshToken_;
		return result;
	}

	Account_ptr Account::Deserialize (const QByteArray& data, QObject* parentPlugin)
	{
		QDataStream str (data);
		quint8 version = 0;
		str >> version;

		if (version != 1)
		{
			qWarning () << Q_FUNC_INFO
					<< "unknown version"
					<< version;
			return Account_ptr ();
		}

		QString name;
		str >> name;
		const auto acc = std::make_shared<Account> (name, parentPlugin);
		str >> acc->Trusted_
				>> acc->RefreshToken_;
		return acc;
	}

	bool Account::IsTrusted () const
	{
		return Trusted_;
	}

	void Account::SetTrusted (bool trust)
	{
		Trusted_ = trust;
	}

	void Account::SetAccessToken (const QString& token)
	{
		AccessToken_ = token;
	}

	void Account::SetRefreshToken (const QString& token)
	{
		RefreshToken_ = token;
	}

	QString Account::GetRefreshToken () const
	{
		return RefreshToken_;
	}

	DriveManager* Account::GetDriveManager () const
	{
		return DriveManager_;
	}

	void Account::handleGotNewItem (const DriveItem& item)
	{
		emit gotNewItem (ToStorageItem (item), item.ParentId_.toUtf8 ());
		emit listingUpdated (item.ParentIsRoot_ ? QByteArray () : item.ParentId_.toUtf8 ());
	}

	void Account::handleGotChanges (const QList<DriveChanges>& driveChanges)
	{
		QList<Change> changes;
		for (const auto& driveChange : driveChanges)
		{
			//TODO setting for shared files
			if (driveChange.FileResource_.PermissionRole_ != DriveItem::Roles::Owner)
				continue;

			Change change;
			change.Deleted_ = driveChange.Deleted_;
			change.ID_ = driveChange.Id_.toUtf8 ();
			change.ItemID_ = driveChange.FileId_.toUtf8 ();
			if (!change.Deleted_)
				change.Item_ = ToStorageItem (driveChange.FileResource_);

			changes << change;
		}

		emit gotChanges (changes);
	}

}
}
}
