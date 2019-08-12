/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "syncer.h"
#include <future>
#include <QCryptographicHash>
#include <QFileInfo>
#include <QStandardItem>
#include <QtDebug>
#include <QUuid>
#include "interfaces/netstoremanager/istorageaccount.h"
#include "utils.h"

namespace LC
{
namespace NetStoreManager
{
	Syncer::Syncer (const QString& dirPath, const QString& remotePath,
			IStorageAccount *isa, QObject *parent)
	: QObject (parent)
	, LocalPath_ (dirPath)
	, RemotePath_ (remotePath)
	, Started_ (false)
	, Account_ (isa)
	, SFLAccount_ (qobject_cast<ISupportFileListings*> (isa->GetQObject ()))
	{
	}

	QByteArray Syncer::GetAccountID () const
	{
		return Account_->GetUniqueID ();
	}

	QString Syncer::GetLocalPath () const
	{
		return LocalPath_;
	}

	QString Syncer::GetRemotePath () const
	{
		return RemotePath_;
	}

	Snapshot_t Syncer::GetSnapshot () const
	{
		return Snapshot_;
	}

	void Syncer::SetSnapshot (const Changes_t& changes)
	{
		Snapshot_.clear ();
		for (const auto& change : changes)
			Snapshot_ [change.ID_] = change;
	}

	bool Syncer::IsStarted () const
	{
		return Started_;
	}

	void Syncer::CreateRemotePath (const QStringList& path)
	{
		if (Id2Path_.right.count (path.join ("/")))
			return;

		QStringList nonExistingPath;
		QStringList existingPath;
		for (int i = path.length () - 1; i >= 0; --i)
		{
			QStringList temp = path.mid (0, i + 1);
			if (Id2Path_.right.count (temp.join ("/")))
			{
				existingPath = temp;
				break;
			}
			else if (nonExistingPath.isEmpty ())
				nonExistingPath = temp;
		}

		if (nonExistingPath.isEmpty ())
			return;

		int lastPos = 0;
		for ( ; lastPos < std::min (existingPath .size (), nonExistingPath.size ()); ++lastPos)
			if (existingPath.at (lastPos) != nonExistingPath.at (lastPos))
				break;

		SFLAccount_->CreateDirectory (nonExistingPath.at (lastPos),
				existingPath.isEmpty () ?
					QByteArray () :
					Id2Path_.right.at (existingPath.join ("/")));
		if (lastPos != nonExistingPath.length () - 1)
			CallsQueue_.append ([this, nonExistingPath] ()
				{ CreateRemotePath (nonExistingPath); });
	}

	void Syncer::DeleteRemotePath (const QStringList& path)
	{
 		SFLAccount_->MoveToTrash ({ Id2Path_.right.at (path.join ("/")) });
	}

	void Syncer::RenameItem (const StorageItem& item, const QString& path)
	{
		const auto& id = item.ID_;
		QString oldPath = Id2Path_.left.at (id);
		if (!item.IsDirectory_)
		{
			auto it = Id2Path_.left.find (id);
			Id2Path_.left.replace_data (it, path);
		}
		else
			for (auto it = Id2Path_.left.begin ();
					it != Id2Path_.left.end (); ++it)
			{
				QString itemPath = it->second;
				if (itemPath.startsWith (oldPath))
				{
					itemPath.replace (oldPath, path);
					Id2Path_.left.replace_data (it, itemPath);
				}
			}
	}

	namespace
	{
		QCryptographicHash::Algorithm NSMHashType2QtCryproHashAlgorithm (HashAlgorithm hash)
		{
			switch (hash)
			{
			case HashAlgorithm::Md4:
				return QCryptographicHash::Md4;
			case HashAlgorithm::Sha1:
				return QCryptographicHash::Sha1;
			case HashAlgorithm::Md5:
			default:
				return QCryptographicHash::Md5;
			}
		}
	}

	Snapshot_t Syncer::CreateSnapshot ()
	{
		Snapshot_t snapshot;

		for (const auto& fi : QDir (LocalPath_).entryInfoList (QDir::NoDotAndDotDot | QDir::AllEntries))
		{
			const QString path = fi.absoluteFilePath ().remove (LocalPath_ + "/");
			Change change;
			StorageItem storage;
			if (!Id2Path_.right.count (path))
				change.ItemID_ = Id2Path_.right.at (path);
			else
			{
				change.ItemID_ = QUuid::createUuid ().toByteArray ();

				storage.IsDirectory_ = fi.isDir ();
				storage.Name_ = fi.fileName ();
				storage.ModifyDate_ = fi.lastModified ();
				storage.ID_ = change.ItemID_;
			}

			if (fi.isFile ())
			{
				QFile file (fi.absoluteFilePath ());
				if (file.open (QIODevice::ReadOnly))
				{
					const auto& ba = file.readAll ();
					storage.Hash_ = QCryptographicHash::hash (ba,
							NSMHashType2QtCryproHashAlgorithm (SFLAccount_->GetCheckSumAlgorithm ()));
					file.close ();
				}
				else
					qWarning () << Q_FUNC_INFO
							<< "unable to open file for hash calculation";

				storage.Size_ = fi.size ();
			}

			change.Item_ = storage;
			snapshot [change.ID_] = change;
		}

		return snapshot;
	}

	Snapshot_t Syncer::CreateDiffSnapshot (const Snapshot_t& newSnapshot,
			const Snapshot_t& oldSnapshot)
	{
		Snapshot_t diffSnapshot;
		return diffSnapshot;
	}

	void Syncer::start ()
	{
		if (Started_)
			return;

		Started_ = true;
		QStringList path = RemotePath_.split ('/');
		CreateRemotePath (path);

		const auto& newSnapshot = CreateSnapshot ();
		//TODO create diff
	}

	void Syncer::stop ()
	{
		CallsQueue_.clear ();
		Started_ = false;
	}

	void Syncer::handleGotItems (const QList<StorageItem>& items)
	{
		Id2Item_.clear ();
		Id2Path_.erase (Id2Path_.begin (), Id2Path_.end ());
		Id2Path_.clear ();
		boost::bimaps::bimap<QByteArray, QStandardItem*, boost::container::allocator<void>> id2StandardItem;
		for (const auto& item : items)
		{
			if (item.IsTrashed_)
				continue;

			Id2Item_ [item.ID_] = item;
			id2StandardItem.insert ({ item.ID_, new QStandardItem (item.Name_) });
		}

		QStandardItem *core = new QStandardItem;
		for (const auto& pair : id2StandardItem.left)
		{
			if (!Id2Item_.contains (Id2Item_ [pair.first].ParentID_))
				core->appendRow (pair.second);
			else
				id2StandardItem.left.at (Id2Item_ [pair.first].ParentID_)->appendRow (pair.second);
		}

		QList<QStandardItem*> parentItems = { core };
		QList<QStandardItem*> childItems;
		while (!parentItems.isEmpty ())
		{
			for (auto item : parentItems)
			{
				for (int i = 0; i < item->rowCount (); ++i)
					childItems << item->child (i);
			}

			for (auto item : childItems)
			{
				const auto& id = id2StandardItem.right.at (item);
				Id2Path_.insert ({ id,
					(Id2Item_.contains (Id2Item_ [id].ParentID_) ?
					(Id2Path_.left.at (Id2Item_ [id].ParentID_) + "/" ) :
					QString ())
						+ item->text () });
			}
			std::swap (parentItems, childItems);
			childItems.clear();
		}

		if (!CallsQueue_.isEmpty ())
			CallsQueue_.dequeue () ();
	}

	void Syncer::handleGotNewItem (const StorageItem& item, const QByteArray& parentId)
	{
		Id2Item_ [item.ID_] = item;
		Id2Path_.insert ({ item.ID_, (Id2Item_.contains (parentId) ?
			(Id2Path_.left.at (parentId) + "/") :
			QString ())
				+ item.Name_ });

		if (!CallsQueue_.isEmpty ())
			CallsQueue_.dequeue () ();
	}

	void Syncer::handleGotChanges (const QList<Change>& changes)
	{
		for (const auto& change : changes)
		{
			const auto& item = change.Item_;
			if (item.IsTrashed_)
				continue;

			const auto& id = change.ItemID_;
			const auto& parentId = change.Item_.ParentID_;

			if (change.Deleted_)
			{
				Id2Item_.remove (id);
				Id2Path_.left.erase (id);
			}
			else if (item.IsValid ())
			{
				QString path = (Id2Item_.contains (parentId) ?
					(Id2Path_.left.at (parentId) + "/") :
					QString ())
						+ item.Name_;
				if (Id2Path_.left.count (id))
				{
					if (Id2Item_ [id].Name_ != item.Name_)
						RenameItem (item, path);
				}
				else
					Id2Path_.insert ({ id, path });
				Id2Item_ [id] = item;
			}
		}

		if (!CallsQueue_.isEmpty ())
			CallsQueue_.dequeue () ();
	}

	void Syncer::localDirWasCreated (const QString& path)
	{
		if (!SFLAccount_)
			return;

		QString dirPath = path;
		QString parentPath = QFileInfo (dirPath).dir ().absolutePath ();
		dirPath.replace (LocalPath_, RemotePath_);
		parentPath.replace (LocalPath_, RemotePath_);

// 		if (Id2Path_.right.count (parentPath))
// 			CreateRemotePath (dirPath.split ("/"));
// 		else
// 			CallsQueue_ << [this, path] () { localDirWasCreated (path); };
	}

	void Syncer::localDirWasRemoved (const QString& path)
	{
		if (!SFLAccount_)
			return;

		QString dirPath = path;
		dirPath.replace (LocalPath_, RemotePath_);
		if (!Id2Path_.right.count (dirPath))
			return;

// 		DeleteRemotePath (dirPath.split ('/'));
	}

	void Syncer::localFileWasCreated (const QString& path)
	{
		QString filePath = path;
		QString dirPath = QFileInfo (filePath).dir ().absolutePath ();
		filePath.replace (LocalPath_, RemotePath_);
		dirPath.replace (LocalPath_, RemotePath_);
		const QByteArray& id = Id2Path_.right.at (dirPath);
// 		if (Id2Path_.right.count (dirPath))
// 			Account_->Upload (path, id);
// 		else
// 			CallsQueue_ << [this, path, id] () { localFileWasCreated (path); };
	}

	void Syncer::localFileWasRemoved (const QString& path)
	{
		if (!SFLAccount_)
			return;

		QString filePath = path;
		filePath.replace (LocalPath_, RemotePath_);
// 		DeleteRemotePath (filePath.split ('/'));
	}

	void Syncer::localFileWasUpdated (const QString& path)
	{
		qDebug () << Q_FUNC_INFO << path;
	}

	void Syncer::localFileWasRenamed (const QString& oldName, const QString& newName)
	{
		QString filePath = oldName;
		filePath.replace (LocalPath_, RemotePath_);
// 		if (Id2Path_.right.count (filePath))
// 			SFLAccount_->Rename (Id2Path_.right.at (filePath),
// 					QFileInfo (newName).fileName ());
// 		else
// 			CallsQueue_ << [this, oldName, newName] ()
// 					{ localFileWasRenamed (oldName, newName); };
	}

}
}
