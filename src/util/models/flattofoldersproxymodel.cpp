/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "flattofoldersproxymodel.h"
#include <algorithm>
#include <QSet>
#include <QMimeData>
#include <util/sll/containerconversions.h>
#include <interfaces/iinfo.h>
#include <interfaces/core/itagsmanager.h>
#include "modelsanitizer.h"

namespace LC::Util
{
	struct FlatTreeItem
	{
		enum class Type : std::uint8_t
		{
			Root,
			Folder,
			Item
		};

		const Type Type_ = Type::Root;
		FlatTreeItem * const Parent_ = nullptr;

		QList<FlatTreeItem_ptr> C_;
		QPersistentModelIndex Index_;
		TagId Tag_;

		FlatTreeItem () = default;

		FlatTreeItem (Type type, FlatTreeItem& parent, TagId tag)
		: Type_ { type }
		, Parent_ { &parent }
		, Tag_ { std::move (tag) }
		{
		}

		int Row () const
		{
			if (!Parent_)
				return 0;

			const auto& siblings = Parent_->C_;
			const auto pos = std::ranges::find_if (siblings, [this] (const auto& sibling) { return sibling.get () == this; });
			return pos - siblings.begin ();
		}

		FlatTreeItem_ptr AddFolder (const TagId& tag)
		{
			return C_.emplaceBack (std::make_shared<FlatTreeItem> (Type::Folder, *this, tag));
		}

		FlatTreeItem_ptr AddLeaf (const QPersistentModelIndex& index)
		{
			const auto& leaf = C_.emplaceBack (std::make_shared<FlatTreeItem> (Type::Item, *this, Tag_));
			leaf->Index_ = index;
			return leaf;
		}
	};

	namespace
	{
		FlatTreeItem* ToFlat (const QModelIndex& idx)
		{
			return static_cast<FlatTreeItem*> (idx.internalPointer ());
		}

		QList<TagId> TagsOf (const QModelIndex& idx)
		{
			auto tags = idx.data (+CustomDataRoles::Tags).value<QList<TagId>> ();
			if (tags.isEmpty ())
				tags << TagId {};
			return tags;
		}
	}

	FlatToFoldersProxyModel::FlatToFoldersProxyModel (const ITagsManager *itm, QObject *parent)
	: QAbstractItemModel { parent }
	, TM_ { itm }
	, Root_ { std::make_shared<FlatTreeItem> () }
	{
		InstallModelSanitizer (*this);
	}

	int FlatToFoldersProxyModel::columnCount (const QModelIndex&) const
	{
		return SourceModel_ ?
			SourceModel_->columnCount ({}) :
			0;
	}

	QVariant FlatToFoldersProxyModel::data (const QModelIndex& index, int role) const
	{
		if (!index.isValid ())
			return {};

		const auto fti = ToFlat (index);

		if (fti->Type_ == FlatTreeItem::Type::Item)
		{
			const auto& source = fti->Index_;
			return source.sibling (source.row (), index.column ()).data (role);
		}

		if (fti->Type_ == FlatTreeItem::Type::Folder && index.column () == 0)
		{
			switch (role)
			{
			case Qt::DisplayRole:
			{
				if (fti->Tag_.Id_.isEmpty ())
					return tr ("untagged");

				if (const auto& tag = TM_->GetTag (fti->Tag_))
					return *tag;
				return tr ("<unknown tag>");
			}
			case +CustomDataRoles::Tags:
				return QVariant::fromValue (fti->Tag_.Id_.isEmpty () ? QList<TagId> {} : QList { fti->Tag_ });
			default:
				return {};
			}
		}

		return {};
	}

	QVariant FlatToFoldersProxyModel::headerData (int section, Qt::Orientation orient, int role) const
	{
		if (!SourceModel_)
			return {};

		return SourceModel_->headerData (section, orient, role);
	}

	Qt::ItemFlags FlatToFoldersProxyModel::flags (const QModelIndex& index) const
	{
		if (!index.isValid ())
			return Qt::ItemIsDropEnabled;

		if (const auto fti = ToFlat (index);
			fti->Type_ == FlatTreeItem::Type::Item)
		{
			const auto& source = fti->Index_;
			return source.sibling (source.row (), index.column ()).flags ();
		}

		return Qt::ItemIsSelectable |
				Qt::ItemIsEnabled |
				Qt::ItemIsDragEnabled |
				Qt::ItemIsDropEnabled;
	}

	bool FlatToFoldersProxyModel::setData (const QModelIndex& index, const QVariant& value, int role)
	{
		if (!index.isValid ())
			return false;

		const auto fti = ToFlat (index);
		if (fti->Type_ != FlatTreeItem::Type::Item)
			return false;

		const auto& source = fti->Index_;
		return SourceModel_->setData (source.sibling (source.row (), index.column ()), value, role);
	}

	QModelIndex FlatToFoldersProxyModel::index (int row, int column, const QModelIndex& parent) const
	{
		if (!hasIndex (row, column, parent))
			return {};

		return createIndex (row, column, ToFlatOrRoot (parent).C_.at (row).get ());
	}

	QModelIndex FlatToFoldersProxyModel::parent (const QModelIndex& index) const
	{
		const auto parent = ToFlatOrRoot (index).Parent_;
		if (!parent || parent->Type_ == FlatTreeItem::Type::Root)
			return {};

		return IndexOf (*parent);
	}

	int FlatToFoldersProxyModel::rowCount (const QModelIndex& index) const
	{
		if (index.column () > 0)
			return 0;

		return ToFlatOrRoot (index).C_.size ();
	}

	Qt::DropActions FlatToFoldersProxyModel::supportedDropActions() const
	{
		return SourceModel_ ?
				SourceModel_->supportedDropActions () :
				QAbstractItemModel::supportedDropActions ();
	}

	QStringList FlatToFoldersProxyModel::mimeTypes() const
	{
		return SourceModel_ ?
				SourceModel_->mimeTypes () :
				QAbstractItemModel::mimeTypes ();
	}

	QMimeData* FlatToFoldersProxyModel::mimeData (const QModelIndexList& indexes) const
	{
		if (!SourceModel_)
			return QAbstractItemModel::mimeData (indexes);

		QModelIndexList sourceIdxs;
		for (const auto& index : indexes)
		{
			auto item = static_cast<FlatTreeItem*> (index.internalPointer ());
			switch (item->Type_)
			{
			case FlatTreeItem::Type::Item:
				sourceIdxs << MapToSource (index);
				break;
			case FlatTreeItem::Type::Folder:
				for (const auto& subItem : item->C_)
					sourceIdxs << subItem->Index_;
				break;
			default:
				break;
			}
		}

		return SourceModel_->mimeData (sourceIdxs);
	}

	bool FlatToFoldersProxyModel::dropMimeData (const QMimeData* data, Qt::DropAction action, int, int, const QModelIndex& parent)
	{
		if (!SourceModel_)
			return false;

		QMimeData modified;
		for (const auto& format : data->formats ())
			modified.setData (format, data->data (format));

		if (parent.isValid ())
			modified.setData (QStringLiteral ("x-leechcraft/tag"), ToFlat (parent)->Tag_.Id_.toLatin1 ());

		return SourceModel_->dropMimeData (&modified, action, -1, -1, QModelIndex ());
	}

	void FlatToFoldersProxyModel::SetSourceModel (QAbstractItemModel *model)
	{
		if (SourceModel_)
			disconnect (SourceModel_,
					nullptr,
					this,
					nullptr);

		SourceModel_ = model;

		if (model)
		{
			// We don't support changing columns (yet) so don't connect
			// to columns* signals.
			// Source layout changes aren't forwarded either: the class contract
			// requires rows to move as units, which leaves nothing to show here.
			connect (model,
					&QAbstractItemModel::headerDataChanged,
					this,
					&QAbstractItemModel::headerDataChanged);
			connect (model,
					&QAbstractItemModel::dataChanged,
					this,
					&FlatToFoldersProxyModel::HandleDataChanged);
			connect (model,
					&QAbstractItemModel::modelReset,
					this,
					&FlatToFoldersProxyModel::HandleModelReset);
			connect (model,
					&QAbstractItemModel::rowsInserted,
					this,
					&FlatToFoldersProxyModel::HandleRowsInserted);
			connect (model,
					&QAbstractItemModel::rowsAboutToBeRemoved,
					this,
					&FlatToFoldersProxyModel::HandleRowsAboutToBeRemoved);
		}

		HandleModelReset ();
	}

	QAbstractItemModel* FlatToFoldersProxyModel::GetSourceModel () const
	{
		return SourceModel_;
	}

	QModelIndex FlatToFoldersProxyModel::MapToSource (const QModelIndex& proxy) const
	{
		if (!GetSourceModel ())
			return {};

		if (!proxy.isValid ())
			return {};

		const auto item = ToFlat (proxy);

		if (item->Type_ != FlatTreeItem::Type::Item)
			return {};

		return item->Index_;
	}

	QList<QModelIndex> FlatToFoldersProxyModel::MapFromSource (const QModelIndex& source) const
	{
		QList<QModelIndex> result;
		for (const auto& leaf : Items_.values (QPersistentModelIndex { source }))
			result << IndexOf (*leaf);
		return result;
	}

	bool FlatToFoldersProxyModel::IsFolder (const QModelIndex& index) const
	{
		const auto item = ToFlat (index);
		return item && item->Type_ == FlatTreeItem::Type::Folder;
	}

	QList<QModelIndex> FlatToFoldersProxyModel::GetChildren (const QModelIndex& index) const
	{
		if (!IsFolder (index))
			return {};

		const auto& children = ToFlat (index)->C_;
		QList<QModelIndex> result;
		result.reserve (children.size ());
		for (const auto& child : children)
			result << child->Index_;
		return result;
	}

	QList<QVariant> FlatToFoldersProxyModel::GetChildrenData (const QModelIndex& index, int role) const
	{
		const auto& children = GetChildren (index);

		QList<QVariant> result;
		result.reserve (children.size ());
		for (const auto& child : children)
			result.push_back (child.data (role));
		return result;
	}

	FlatTreeItem_ptr FlatToFoldersProxyModel::FindFolder (const TagId& tag) const
	{
		const auto& folders = Root_->C_;
		const auto pos = std::ranges::find (folders, tag, &FlatTreeItem::Tag_);
		return pos == folders.end () ? nullptr : *pos;
	}

	FlatTreeItem_ptr FlatToFoldersProxyModel::GetFolder (const TagId& tag)
	{
		if (const auto& folder = FindFolder (tag))
			return folder;

		const auto row = Root_->C_.size ();
		beginInsertRows ({}, row, row);
		const auto& folder = Root_->AddFolder (tag);
		endInsertRows ();
		return folder;
	}

	const FlatTreeItem& FlatToFoldersProxyModel::ToFlatOrRoot (const QModelIndex& idx) const
	{
		return idx.isValid () ? *ToFlat (idx) : *Root_;
	}

	QModelIndex FlatToFoldersProxyModel::IndexOf (const FlatTreeItem& item, int column) const
	{
		return createIndex (item.Row (), column, &item);
	}

	void FlatToFoldersProxyModel::AddForTag (const TagId& tag, const QPersistentModelIndex& pidx)
	{
		const auto& folder = GetFolder (tag);
		const auto row = folder->C_.size ();
		beginInsertRows (IndexOf (*folder), row, row);
		Items_.insert (pidx, folder->AddLeaf (pidx));
		endInsertRows ();
	}

	void FlatToFoldersProxyModel::RemoveLeaf (const FlatTreeItem_ptr& leaf)
	{
		const auto folder = leaf->Parent_;
		const auto leafRow = leaf->Row ();
		beginRemoveRows (IndexOf (*folder), leafRow, leafRow);
		Items_.remove (leaf->Index_, leaf);
		folder->C_.removeAt (leafRow);
		endRemoveRows ();

		if (!folder->C_.isEmpty ())
			return;

		const auto folderRow = folder->Row ();
		beginRemoveRows ({}, folderRow, folderRow);
		Root_->C_.removeAt (folderRow);
		endRemoveRows ();
	}

	void FlatToFoldersProxyModel::HandleDataChanged (const QModelIndex& topLeft, const QModelIndex& bottomRight, const QList<int>& roles)
	{
		for (int row = topLeft.row (); row <= bottomRight.row (); ++row)
		{
			const QPersistentModelIndex pidx { topLeft.sibling (row, 0) };
			auto tags = AsSet (TagsOf (pidx));
			for (const auto& leaf : Items_.values (pidx))
				if (tags.remove (leaf->Tag_))
					emit dataChanged (IndexOf (*leaf), IndexOf (*leaf, columnCount () - 1), roles);
				else
					RemoveLeaf (leaf);

			for (const auto& tag : tags)
				AddForTag (tag, pidx);
		}
	}

	void FlatToFoldersProxyModel::HandleModelReset ()
	{
		beginResetModel ();
		Items_.clear ();
		Root_->C_.clear ();
		if (SourceModel_)
			for (int i = 0, size = SourceModel_->rowCount (); i < size; ++i)
			{
				const QPersistentModelIndex pidx { SourceModel_->index (i, 0) };
				for (const auto& tag : TagsOf (pidx))
				{
					auto folder = FindFolder (tag);
					if (!folder)
						folder = Root_->AddFolder (tag);
					Items_.insert (pidx, folder->AddLeaf (pidx));
				}
			}
		endResetModel ();
	}

	void FlatToFoldersProxyModel::HandleRowsInserted (const QModelIndex&, int start, int end)
	{
		for (int i = start; i <= end; ++i)
		{
			const QPersistentModelIndex pidx { SourceModel_->index (i, 0) };
			for (const auto& tag : TagsOf (pidx))
				AddForTag (tag, pidx);
		}
	}

	void FlatToFoldersProxyModel::HandleRowsAboutToBeRemoved (const QModelIndex&, int start, int end)
	{
		for (int i = start; i <= end; ++i)
			for (const auto& leaf : Items_.values (QPersistentModelIndex { SourceModel_->index (i, 0) }))
				RemoveLeaf (leaf);
	}
}
