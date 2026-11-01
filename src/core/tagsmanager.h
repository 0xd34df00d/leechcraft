/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <flat_set>
#include <memory>
#include <optional>
#include <variant>
#include <QAbstractItemModel>
#include <QHash>
#include <QUuid>
#include <QString>
#include <QMetaType>
#include <util/sll/either.h>
#include <util/sll/void.h>
#include "interfaces/core/itagsmanager.h"
#include "tagsstorage.h"

namespace LC
{
	class TagsManager final : public QAbstractItemModel
							, public ITagsManager
	{
		Q_OBJECT
		Q_INTERFACES (ITagsManager)

		QHash<QUuid, std::flat_set<QString>> Names_;
		QStringList Rows_;
		std::unique_ptr<TagsStorage> Storage_;
	public:
		explicit TagsManager (std::unique_ptr<TagsStorage>);

		static TagsManager& Instance ();

		int columnCount (const QModelIndex&) const override;
		QVariant data (const QModelIndex&, int) const override;
		QModelIndex index (int, int, const QModelIndex&) const override;
		QModelIndex parent (const QModelIndex&) const override;
		int rowCount (const QModelIndex&) const override;

		std::optional<TagId> GetID (const QString&) override;
		using ITagsManager::GetTags;
		QStringList GetTags (const TagId&) const override;
		QStringList GetAllTags () const override;
		QStringList Split (const QString&) const override;
		QList<TagId> SplitToIDs (const QString&) override;
		QString Join (const QStringList&) const override;
		QString JoinIDs (const QList<TagId>&) const override;
		QAbstractItemModel* GetModel () override;

		struct BlankTagName {};
		struct DelimiterInTagName
		{
			QString Delimiter_;
		};
		using InvalidTagName = std::variant<BlankTagName, DelimiterInTagName>;

		/** @brief Replaces the name `old` with the trimmed `newNames` on
		 * every tag having it.
		 *
		 * One new name renames the tag, several names split it, a name
		 * that already exists merges it into that tag, and no names at all
		 * remove it.
		 */
		Util::Either<InvalidTagName, Util::Void> ReplaceName (const QString& old, const QStringList& newNames);
	private:
		void MigrateStoredNames ();
		TagId InsertTag (const QString&);
		void SetRows (const QStringList&);
	};
}
