/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QStringList>
#include <QAbstractItemModel>
#include <QList>
#include "messagelistactioninfo.h"

namespace LC
{
namespace Snails
{
	class MessageListActionsManager;
	struct MessageInfo;

	class MailModel : public QAbstractItemModel
	{
		Q_OBJECT

		const MessageListActionsManager * const ActionsMgr_;

		const QStringList Headers_;

		QStringList Folder_;

		struct TreeNode;
		typedef std::shared_ptr<TreeNode> TreeNode_ptr;
		typedef std::weak_ptr<TreeNode> TreeNode_wptr;
		const TreeNode_ptr Root_;

		QHash<QByteArray, QList<TreeNode_ptr>> FolderId2Nodes_;
		QHash<QByteArray, QByteArray> MsgId2FolderId_;

		mutable QHash<QByteArray, QList<MessageListActionInfo>> MsgId2Actions_;
	public:
		enum class Column
		{
			From,
			UnreadChildren,
			StatusIcon,
			Subject,
			Date
		};
		static const Column MaxColumn = Column::Date;

		enum MailRole
		{
			ID = Qt::UserRole + 1,
			Sort,
			IsRead,
			UnreadChildrenCount,
			TotalChildrenCount,
			MessageActions,
			MsgInfo
		};

		MailModel (const MessageListActionsManager*, QObject* = 0);

		QVariant headerData (int, Qt::Orientation, int) const override;
		int columnCount (const QModelIndex& = {}) const override;
		QVariant data (const QModelIndex&, int) const override;
		Qt::ItemFlags flags (const QModelIndex&) const override;
		QModelIndex index (int, int, const QModelIndex& = {}) const override;
		QModelIndex parent (const QModelIndex&) const override;
		int rowCount (const QModelIndex& = {}) const override;
		bool setData (const QModelIndex&, const QVariant&, int) override;

		QStringList mimeTypes () const override;
		QMimeData* mimeData (const QModelIndexList&) const override;
		Qt::DropActions supportedDragActions () const override;

		void SetFolder (const QStringList&);
		QStringList GetCurrentFolder () const;

		void Clear ();

		void Append (QList<MessageInfo>);
		bool Remove (const QByteArray&);

		void UpdateReadStatus (const QList<QByteArray>& msgIds, bool read);

		void MarkUnavailable (const QList<QByteArray>&);

		QList<QByteArray> GetCheckedIds () const;
		bool HasCheckedIds () const;
	private:
		void UpdateParents (const QByteArray&, bool);

		void RemoveNode (const TreeNode_ptr&);
		bool AppendStructured (const MessageInfo&);

		void EmitRowChanged (const TreeNode_ptr&);

		QModelIndex GetIndex (const TreeNode_ptr& node, int column) const;
		QList<QModelIndex> GetIndexes (const QByteArray& folderId, int column) const;
		QList<QList<QModelIndex>> GetIndexes (const QByteArray& folderId, const QList<int>& columns) const;
	signals:
		void messageListUpdated ();
		void messagesSelectionChanged ();
	};
}
}
