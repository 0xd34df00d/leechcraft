/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <unordered_map>
#include <filesystem>
#include <QIcon>
#include <QString>
#include <QAbstractItemModel>
#include <util/models/modelitembase.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/iiconthememanager.h>
#include "core.h"

namespace LC
{
namespace BitTorrent
{
	template<typename T>
	struct TorrentNodeInfoBase : public Util::ModelItemBase<T>
	{
		QIcon Icon_;

		QString Name_;

		// (ColumnPath, RawDataRole)
		std::filesystem::path ParentPath_;

		// (ColumnPath, RoleFileIndex)
		int FileIndex_ = -1;

		// (ColumnPath, RoleSize)
		// (2, RoleSize)
		// (ColumnProgress, RawDataRole)
		qulonglong SubtreeSize_ = 0;

		TorrentNodeInfoBase (const std::shared_ptr<T>& parent)
		: Util::ModelItemBase<T> { parent }
		{
		}

		std::filesystem::path GetFullPath () const
		{
			return ParentPath_ / Name_.toStdString ();
		}

		QString GetFullPathStr () const
		{
			return QString::fromUtf8 (GetFullPath ().string ().c_str ());
		}

		void Reparent (const std::shared_ptr<T>& newParent)
		{
			this->Parent_ = newParent;
		}
	};

	template<typename T>
	using TorrentNodeInfoBase_ptr = std::shared_ptr<TorrentNodeInfoBase<T>>;

	template<typename T>
	class TorrentFilesModelBase : public QAbstractItemModel
	{
		const QStringList HeaderData_;

		struct Hash
		{
			size_t operator() (const std::filesystem::path& path) const
			{
				return std::hash<std::string> {} (path.native ());
			}
		};
	protected:
		using Path2Node_t = std::unordered_map<std::filesystem::path, std::shared_ptr<T>, Hash>;
		Path2Node_t Path2Node_;

		const std::shared_ptr<T> RootNode_;

		std::filesystem::path BasePath_;

		int FilesInTorrent_ = 0;

		TorrentFilesModelBase (const QStringList& headers, QObject *parent = nullptr)
		: QAbstractItemModel { parent }
		, HeaderData_ { headers }
		, RootNode_ { std::make_shared<T> (std::shared_ptr<T> {}) }
		{
		}

		QModelIndex IndexForNode (T *node, int column = 0) const
		{
			return createIndex (node->GetRow (), column, node);
		}

		QModelIndex IndexForNode (const std::shared_ptr<T>& node, int column = 0) const
		{
			return IndexForNode (node.get (), column);
		}

		QModelIndex FindIndex (const std::filesystem::path& path) const
		{
			if (path.empty ())
				return {};

			const auto pos = Path2Node_.find (path);
			if (pos == Path2Node_.end ())
				throw std::runtime_error ("TorrentFilesModelBase::FindIndex(): unknown path " + path.string ());

			return IndexForNode (pos->second);
		}

		const std::shared_ptr<T>& MkParentIfDoesntExist (const std::filesystem::path& path, bool announce = false)
		{
			const auto& parentPath = path.parent_path ();
			const auto pos = Path2Node_.find (parentPath);
			if (pos != Path2Node_.end ())
				return pos->second;

			const auto& parent = MkParentIfDoesntExist (parentPath);

			const auto& parentParentPath = parentPath.parent_path ();
			if (announce)
				beginInsertRows (FindIndex (parentParentPath), parent->GetRowCount (), parent->GetRowCount ());
			const auto& node = parent->AppendChild (parent);
			node->ParentPath_ = parentParentPath;
			node->Name_ = QString::fromStdU16String (parentPath.filename ().u16string ());
			node->Icon_ = Core::Instance ()->GetProxy ()->
					GetIconThemeManager ()->GetIcon ("document-open-folder");

			const auto& insertResult = Path2Node_.insert ({ parentPath, node });
			if (announce)
				endInsertRows ();

			return insertResult.first->second;
		}
	public:
		int columnCount (const QModelIndex&) const override final
		{
			return HeaderData_.size ();
		}

		QVariant headerData (int h, Qt::Orientation orient, int role) const override final
		{
			if (orient == Qt::Horizontal && role == Qt::DisplayRole)
				return HeaderData_.value (h);

			return {};
		}

		QModelIndex index (int row, int col, const QModelIndex& parent = {}) const override final
		{
			if (!hasIndex (row, col, parent))
				return {};

			const auto nodePtr = parent.isValid () ?
					static_cast<T*> (parent.internalPointer ()) :
					RootNode_.get ();

			if (const auto childNode = nodePtr->GetChild (row))
				return createIndex (row, col, childNode.get ());
			return {};
		}

		QModelIndex parent (const QModelIndex& index) const override final
		{
			if (!index.isValid ())
				return {};

			const auto nodePtr = static_cast<T*> (index.internalPointer ());
			const auto parent = nodePtr->GetParent ();

			if (parent == RootNode_)
				return {};

			return IndexForNode (parent);
		}

		int rowCount (const QModelIndex& parent) const override final
		{
			const auto nodePtr = parent.isValid () ?
					static_cast<T*> (parent.internalPointer ()) :
					RootNode_.get ();
			return nodePtr->GetRowCount ();
		}

		void Clear ()
		{
			if (!RootNode_->GetRowCount ())
				return;

			BasePath_.clear ();

			beginRemoveRows ({}, 0, RootNode_->GetRowCount () - 1);
			RootNode_->EraseChildren (RootNode_->begin (), RootNode_->end ());
			endRemoveRows ();

			FilesInTorrent_ = 0;
			Path2Node_.clear ();
		}
	};
}
}
