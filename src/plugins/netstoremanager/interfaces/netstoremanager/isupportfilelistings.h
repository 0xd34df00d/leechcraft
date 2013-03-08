/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
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

#ifndef PLUGINS_NETSTOREMANAGER_INTERFACES_NETSTOREMANAGER_ISUPPORTFILELISTINGS_H
#define PLUGINS_NETSTOREMANAGER_INTERFACES_NETSTOREMANAGER_ISUPPORTFILELISTINGS_H
#include <QStringList>
#include <QtPlugin>
#include <QUrl>

class QStandardItem;

namespace LeechCraft
{
namespace NetStoreManager
{
	enum ListingRole
	{
		ID = Qt::UserRole + 20,
		InTrash,
		Directory,
		ModifiedDate,
		Hash
	};

	enum ListingOp
	{
		NoneOp = 0x00,
		Delete = 0x01,
		TrashSupporting = 0x02,
		DirectorySupport = 0x04
	};

	Q_DECLARE_FLAGS (ListingOps, ListingOp);

	struct Change
	{
		QStringList Id_;
		bool Deleted_;
		QList<QStandardItem*> Row_;

		QStringList ParentId_;
		bool ParentIsRoot_;
	};

	class ISupportFileListings
	{
	public:
		virtual ~ISupportFileListings () {}

		virtual ListingOps GetListingOps () const = 0;

		virtual void RefreshListing () = 0;
		virtual QStringList GetListingHeaders () const = 0;

		virtual void Delete (const QList<QStringList>& id, bool ask = true) = 0;
		virtual void MoveToTrash (const QList<QStringList>& id) = 0;
		virtual void RestoreFromTrash (const QList<QStringList>& id) = 0;
		virtual void EmptyTrash (const QList<QStringList>& id) = 0;
		virtual void RequestUrl (const QList<QStringList>& id) = 0;
		virtual void CreateDirectory (const QString& name, const QStringList& parentId) = 0;
		virtual void Copy (const QStringList& id, const QStringList& newParentId) = 0;
		virtual void Move (const QStringList& id, const QStringList& newParentId) = 0;
		virtual void Rename (const QStringList& id, const QString& newName) = 0;
		virtual void RequestChanges () = 0;

	protected:
		virtual void gotListing (const QList<QList<QStandardItem*>>&) = 0;
		virtual void gotFileUrl (const QUrl& url, const QStringList& id) = 0;
		virtual void gotChanges (const QList<Change>& changes) = 0;
		virtual void gotNewItem (const QList<QStandardItem*>& item, const QStringList& parentId) = 0;
	};
}
}

Q_DECLARE_INTERFACE (LeechCraft::NetStoreManager::ISupportFileListings,
		"org.Deviant.LeechCraft.NetStoreManager.ISupportFileListings/1.0");
Q_DECLARE_OPERATORS_FOR_FLAGS (LeechCraft::NetStoreManager::ListingOps);

#endif
