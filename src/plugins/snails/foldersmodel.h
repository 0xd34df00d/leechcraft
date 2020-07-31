/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <array>
#include <optional>
#include <QAbstractItemModel>
#include <QStringList>
#include "common.h"
#include "folder.h"

namespace LC
{
namespace Snails
{
	class Account;
	struct Folder;

	struct FolderDescr;
	typedef std::shared_ptr<FolderDescr> FolderDescr_ptr;

	class FoldersModel : public QAbstractItemModel
	{
		Q_OBJECT

		Account * const Acc_;

		const QStringList Headers_;

		FolderDescr_ptr RootFolder_;
		QHash<QStringList, FolderDescr*> Folder2Descr_;

		std::array<QStringList, static_cast<int> (FolderType::Other)> CustomFolders_;
	public:
		enum Role
		{
			FolderPath = Qt::UserRole + 1
		};

		enum Column
		{
			FolderName,
			MessageCount
		};

		FoldersModel (Account*);

		QVariant headerData (int, Qt::Orientation, int) const override;
		int columnCount (const QModelIndex& = {}) const override;
		QVariant data (const QModelIndex&, int) const override;
		Qt::ItemFlags flags (const QModelIndex& index) const override;
		QModelIndex index (int, int, const QModelIndex& = {}) const override;
		QModelIndex parent (const QModelIndex&) const override;
		int rowCount (const QModelIndex& = {}) const override;

		QStringList mimeTypes () const override;
		bool canDropMimeData (const QMimeData*, Qt::DropAction, int, int, const QModelIndex&) const override;
		bool dropMimeData (const QMimeData*, Qt::DropAction, int, int, const QModelIndex&) override;
		Qt::DropActions supportedDropActions () const override;

		void SetFolders (const QList<Folder>& folders);
		void SetFolderCounts (const QStringList&, int unread, int total);

		std::optional<QStringList> GetFolderPath (FolderType) const;
	signals:
		void msgMoveRequested (const QList<QByteArray>& ids,
				const QStringList& from, const QStringList& to, Qt::DropAction action);
	};
}
}
