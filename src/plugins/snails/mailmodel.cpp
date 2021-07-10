/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "mailmodel.h"
#include <QIcon>
#include <QMimeData>
#include <QtConcurrentMap>
#include <util/util.h>
#include <util/sll/prelude.h>
#include <util/models/modelitembase.h>
#include <interfaces/core/iiconthememanager.h>
#include "core.h"
#include "messagelistactionsmanager.h"
#include "common.h"
#include "messageinfo.h"

namespace LC
{
namespace Snails
{
	struct MailModel::TreeNode : Util::ModelItemBase<MailModel::TreeNode>
	{
		MessageInfo Msg_;

		QSet<QByteArray> UnreadChildren_;

		bool IsChecked_ = false;

		TreeNode () = default;

		TreeNode (const MessageInfo& msg, const TreeNode_ptr& parent)
		: Util::ModelItemBase<TreeNode> { parent }
		, Msg_ { msg }
		{
		}

		void SetParent (const TreeNode_wptr& parent)
		{
			Parent_ = parent;
		}

		template<typename G, typename F>
		std::result_of_t<G (const TreeNode*)> Fold (const G& g, const F& f) const
		{
			const auto& values = Util::Map (Children_, [&] (const TreeNode_ptr& c) { return c->Fold (g, f); });
			return std::accumulate (values.begin (), values.end (), g (this), f);
		}
	};

	MailModel::MailModel (const MessageListActionsManager *actsMgr, QObject *parent)
	: QAbstractItemModel { parent }
	, ActionsMgr_ { actsMgr }
	, Headers_ { tr ("From"), {}, {}, tr ("Subject"), tr ("Date"), tr ("Size") }
	, Folder_ { "INBOX" }
	, Root_ { std::make_shared<TreeNode> () }
	{
	}

	QVariant MailModel::headerData (int section, Qt::Orientation orient, int role) const
	{
		if (orient != Qt::Horizontal || role != Qt::DisplayRole)
			return {};

		return Headers_.value (section);
	}

	int MailModel::columnCount (const QModelIndex&) const
	{
		return Headers_.size ();
	}

	namespace
	{
		QString GetNeatDate (QDateTime dt)
		{
			dt = dt.toLocalTime ();
			const auto& now = QDateTime::currentDateTime ();

			const auto days = dt.daysTo (now);

			if (!days)
				return dt.time ().toString ("HH:mm");

			if (days == 1)
				return MailModel::tr ("yesterday, %1")
						.arg (dt.time ().toString ("HH:mm"));

			if (dt.date ().weekNumber () == now.date ().weekNumber ())
				return dt.toString ("dddd, HH:mm");

			if (dt.date ().year () == now.date ().year ())
				return dt.date ().toString ("dd MMMM");

			return QLocale {}.toString (dt.date (), QLocale::ShortFormat);
		}
	}

	QVariant MailModel::data (const QModelIndex& index, int role) const
	{
		const auto structItem = static_cast<TreeNode*> (index.internalPointer ());
		if (structItem == Root_.get ())
			return {};

		const auto& msg = structItem->Msg_;

		const auto column = static_cast<Column> (index.column ());

		switch (role)
		{
		case MessageActions:
			if (!MsgId2Actions_.contains (msg.FolderId_))
				MsgId2Actions_ [msg.FolderId_] = ActionsMgr_->GetMessageActions (msg);
			return QVariant::fromValue (MsgId2Actions_.value (msg.FolderId_));
		case Qt::DisplayRole:
		case Sort:
			break;
		case Qt::CheckStateRole:
			return structItem->IsChecked_ ?
					Qt::Checked :
					Qt::Unchecked;
		case Qt::DecorationRole:
		{
			QString iconName;
			if (!msg.IsRead_)
				iconName = "mail-unread-new";
			else if (!structItem->UnreadChildren_.empty ())
				iconName = "mail-unread";
			else
				iconName = "mail-read";

			return Core::Instance ().GetProxy ()->GetIconThemeManager ()->GetIcon (iconName);
		}
		case ID:
			return msg.FolderId_;
		case IsRead:
			return msg.IsRead_;
		case UnreadChildrenCount:
			return structItem->UnreadChildren_.size ();
		case TotalChildrenCount:
			return structItem->Fold ([] (const TreeNode *n) { return n->GetRowCount (); },
					[] (int a, int b) { return a + b; });
		case MsgInfo:
			return QVariant::fromValue (msg);
		default:
			return {};
		}

		switch (column)
		{
		case Column::From:
		{
			const auto& addr = msg.Addresses_ [AddressType::From].value (0);
			return addr.Name_.isEmpty () ? addr.Email_ : addr.Name_;
		}
		case Column::Subject:
		{
			const auto& subject = msg.Subject_;
			return subject.isEmpty () ? "<" + tr ("No subject") + ">" : subject;
		}
		case Column::Date:
		{
			const auto& date = role != Sort || index.parent ().isValid () ?
						msg.Date_ :
						structItem->Fold ([] (const TreeNode *n) { return n->Msg_.Date_; },
								[] (auto d1, auto d2) { return std::max (d1, d2); });
			if (role == Sort)
				return date;
			else
				return GetNeatDate (date);
		}
		case Column::UnreadChildren:
			if (const auto unread = structItem->UnreadChildren_.size ())
				return unread;

			return role == Sort ? QVariant { 0 } : QVariant { QStringLiteral ("·") };
		case Column::StatusIcon:
			break;
		}

		return {};
	}

	Qt::ItemFlags MailModel::flags (const QModelIndex& index) const
	{
		auto flags = QAbstractItemModel::flags (index);
		flags |= Qt::ItemIsDragEnabled;
		if (index.column () == static_cast<int> (Column::Subject))
			flags |= Qt::ItemIsEditable;
		return flags;
	}

	QModelIndex MailModel::index (int row, int column, const QModelIndex& parent) const
	{
		const auto structItem = parent.isValid () ?
				static_cast<TreeNode*> (parent.internalPointer ()) :
				Root_.get ();
		const auto childItem = structItem->GetChild (row).get ();
		if (!childItem)
			return {};

		return createIndex (row, column, childItem);
	}

	QModelIndex MailModel::parent (const QModelIndex& index) const
	{
		if (!index.isValid ())
			return {};

		const auto structItem = static_cast<TreeNode*> (index.internalPointer ());
		const auto& parentItem = structItem->GetParent ();
		if (parentItem == Root_)
			return {};

		return createIndex (parentItem->GetRow (), 0, parentItem.get ());
	}

	int MailModel::rowCount (const QModelIndex& parent) const
	{
		const auto structItem = parent.isValid () ?
				static_cast<TreeNode*> (parent.internalPointer ()) :
				Root_.get ();
		return structItem->GetRowCount ();
	}

	bool MailModel::setData (const QModelIndex& index, const QVariant& value, int role)
	{
		if (role != Qt::CheckStateRole)
			return false;

		if (!index.isValid ())
			return false;

		const auto structItem = static_cast<TreeNode*> (index.internalPointer ());
		structItem->IsChecked_ = value.toInt () == Qt::Checked;

		emit dataChanged (index, index);

		emit messagesSelectionChanged ();

		return true;
	}

	QStringList MailModel::mimeTypes () const
	{
		return
		{
			Mimes::FolderPath,
			Mimes::MessageIdList
		};
	}

	QMimeData* MailModel::mimeData (const QModelIndexList& indexes) const
	{
		if (indexes.isEmpty ())
			return nullptr;

		QByteArray idsData;
		{
			QList<QByteArray> idsList;
			for (const auto& index : indexes)
				idsList << index.data (MailModel::MailRole::ID).toByteArray ();

			QDataStream ostr { &idsData, QIODevice::WriteOnly };
			ostr << idsList;
		}

		auto data = new QMimeData;
		data->setData (Mimes::FolderPath, GetCurrentFolder ().join ('/').toUtf8 ());
		data->setData (Mimes::MessageIdList, idsData);

		return data;
	}

	Qt::DropActions MailModel::supportedDragActions () const
	{
		return Qt::CopyAction | Qt::MoveAction;
	}

	void MailModel::SetFolder (const QStringList& folder)
	{
		Folder_ = folder;
	}

	QStringList MailModel::GetCurrentFolder () const
	{
		return Folder_;
	}

	void MailModel::Clear ()
	{
		auto rc = rowCount ();
		if (!rc)
			return;

		beginRemoveRows ({}, 0, rc - 1);
		Root_->EraseChildren (Root_->begin (), Root_->end ());
		FolderId2Nodes_.clear ();
		MsgId2FolderId_.clear ();
		endRemoveRows ();

		MsgId2Actions_.clear ();
	}

	void MailModel::Append (QList<MessageInfo> messages)
	{
		if (messages.isEmpty ())
			return;

		for (auto i = messages.begin (); i != messages.end (); )
		{
			const auto& msg = *i;

			if (msg.Folder_ != Folder_)
			{
				i = messages.erase (i);
				continue;
			}

			++i;
		}

		if (messages.isEmpty ())
			return;

		std::stable_sort (messages.begin (), messages.end (), Util::ComparingBy (&MessageInfo::Date_));

		for (const auto& msg : messages)
		{
			const auto& msgId = msg.MessageId_;
			if (!msgId.isEmpty ())
				MsgId2FolderId_ [msgId] = msg.FolderId_;
		}

		for (const auto& msg : messages)
			if (!AppendStructured (msg))
			{
				const auto node = std::make_shared<TreeNode> (msg, Root_);

				const auto childrenCount = Root_->GetRowCount ();
				beginInsertRows ({}, childrenCount, childrenCount);
				Root_->AppendExisting (node);
				FolderId2Nodes_ [msg.FolderId_] << node;
				endInsertRows ();
			}

		emit messageListUpdated ();
	}

	bool MailModel::Remove (const QByteArray& id)
	{
		UpdateParents (id, true);

		for (const auto& node : FolderId2Nodes_.value (id))
			RemoveNode (node);

		FolderId2Nodes_.remove (id);
		MsgId2FolderId_.remove (MsgId2FolderId_.key (id));

		return true;
	}

	void MailModel::UpdateReadStatus (const QList<QByteArray>& msgIds, bool read)
	{
		for (const auto& msgId : msgIds)
		{
			for (const auto& indexPair : GetIndexes (msgId, { 0, columnCount () - 1 }))
			{
				for (const auto& index : indexPair)
					static_cast<TreeNode*> (index.internalPointer ())->Msg_.IsRead_ = read;

				emit dataChanged (indexPair.value (0), indexPair.value (1));
			}

			UpdateParents (msgId, read);
		}
	}

	void MailModel::MarkUnavailable (const QList<QByteArray>& ids)
	{
		for (const auto& id : ids)
			Remove (id);
	}

	QList<QByteArray> MailModel::GetCheckedIds () const
	{
		QList<QByteArray> result;

		for (const auto& list : FolderId2Nodes_)
			for (const auto& node : list)
				if (node->IsChecked_)
					result << node->Msg_.FolderId_;

		std::sort (result.begin (), result.end ());
		result.erase (std::unique (result.begin (), result.end ()), result.end ());

		return result;
	}

	bool MailModel::HasCheckedIds () const
	{
		return std::any_of (FolderId2Nodes_.begin (), FolderId2Nodes_.end (),
				[] (const auto& list)
				{
					return std::any_of (list.begin (), list.end (), [] (const auto& node) { return node->IsChecked_; });
				});
	}

	void MailModel::UpdateParents (const QByteArray& folderId, bool read)
	{
		QList<TreeNode_ptr> nodes;
		for (const auto& node : FolderId2Nodes_.value (folderId))
		{
			const auto& parent = node->GetParent ();
			if (parent != Root_)
				nodes << parent;
		}

		for (int i = 0; i < nodes.size (); ++i)
		{
			const auto& item = nodes.at (i);

			if (!read && !item->UnreadChildren_.contains (folderId))
				item->UnreadChildren_ << folderId;
			else if (read)
				item->UnreadChildren_.remove (folderId);

			const auto& leftIdx = createIndex (item->GetRow (), 0, item.get ());
			const auto& rightIdx = createIndex (item->GetRow (), columnCount () - 1, item.get ());
			emit dataChanged (leftIdx, rightIdx);

			const auto& parent = item->GetParent ();
			if (parent != Root_)
				nodes << parent;
		}
	}

	void MailModel::RemoveNode (const TreeNode_ptr& node)
	{
		const auto& parent = node->GetParent ();

		const auto& parentIndex = parent == Root_ ?
				QModelIndex {} :
				createIndex (parent->GetRow (), 0, parent.get ());

		const auto row = node->GetRow ();

		if (const auto childCount = node->GetRowCount ())
		{
			const auto& nodeIndex = createIndex (row, 0, node.get ());

			beginRemoveRows (nodeIndex, 0, childCount - 1);
			auto childNodes = std::move (node->GetChildren ());
			endRemoveRows ();

			for (const auto& childNode : childNodes)
				childNode->SetParent (parent);

			beginInsertRows (parentIndex,
					parent->GetRowCount (),
					parent->GetRowCount () + childCount - 1);
			parent->AppendExisting (childNodes);
			endInsertRows ();
		}

		beginRemoveRows (parentIndex, row, row);
		parent->EraseChild (parent->begin () + row);
		endRemoveRows ();
	}

	bool MailModel::AppendStructured (const MessageInfo& msg)
	{
		auto refs = msg.References_;
		for (const auto& replyTo : msg.InReplyTo_)
			if (!refs.contains (replyTo))
				refs << replyTo;

		if (refs.isEmpty ())
			return false;

		QByteArray folderId;
		for (int i = refs.size () - 1; i >= 0; --i)
		{
			folderId = MsgId2FolderId_.value (refs.at (i));
			if (!folderId.isEmpty ())
				break;
		}
		if (folderId.isEmpty ())
			return false;

		const auto& indexes = GetIndexes (folderId, 0);
		for (const auto& parentIndex : indexes)
		{
			const auto parentNode = static_cast<TreeNode*> (parentIndex.internalPointer ());
			const auto row = parentNode->GetRowCount ();

			const auto node = std::make_shared<TreeNode> (msg, parentNode->shared_from_this ());
			beginInsertRows (parentIndex, row, row);
			parentNode->AppendExisting (node);
			FolderId2Nodes_ [msg.FolderId_] << node;
			endInsertRows ();
		}

		UpdateParents (msg.FolderId_, msg.IsRead_);

		return !indexes.isEmpty ();
	}

	void MailModel::EmitRowChanged (const TreeNode_ptr& node)
	{
		emit dataChanged (GetIndex (node, 0),
				GetIndex (node, static_cast<int> (MaxColumn)));
	}

	QModelIndex MailModel::GetIndex (const TreeNode_ptr& node, int column) const
	{
		return createIndex (node->GetRow (), column, node.get ());
	}

	QList<QModelIndex> MailModel::GetIndexes (const QByteArray& folderId, int column) const
	{
		return Util::Map (FolderId2Nodes_.value (folderId),
				[this, column] (const auto& node) { return GetIndex (node, column); });
	}

	QList<QList<QModelIndex>> MailModel::GetIndexes (const QByteArray& folderId, const QList<int>& columns) const
	{
		return Util::Map (FolderId2Nodes_.value (folderId),
				[this, &columns] (const auto& node)
				{
					return Util::Map (columns,
							[this, &node] (auto column) { return GetIndex (node, column); });
				});
	}
}
}
