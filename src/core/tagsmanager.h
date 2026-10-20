/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <optional>
#include <variant>
#include <QAbstractItemModel>
#include <QMap>
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

		TagsManager ();
	public:
		typedef QMap<QUuid, QString> TagsDictionary_t;
	private:
		TagsDictionary_t Tags_;
		TagsStorage Storage_;
	public:
		static TagsManager& Instance ();

		int columnCount (const QModelIndex&) const override;
		QVariant data (const QModelIndex&, int) const override;
		QModelIndex index (int, int, const QModelIndex&) const override;
		QModelIndex parent (const QModelIndex&) const override;
		int rowCount (const QModelIndex&) const override;

		std::optional<tag_id> GetID (const QString&) override;
		std::optional<QString> GetTag (tag_id) const override;
		QStringList GetAllTags () const override;
		QStringList Split (const QString&) const override;
		QList<tag_id> SplitToIDs (const QString&) override;
		QString Join (const QStringList&) const override;
		QString JoinIDs (const QStringList&) const override;
		QAbstractItemModel* GetModel () override;
		QObject* GetQObject () override;

		void RemoveTag (const QModelIndex&);

		struct BlankTagName {};
		struct DelimiterInTagName
		{
			QString Delimiter_;
		};
		struct TagNameTaken
		{
			QString Name_;
		};
		using RenameError = std::variant<BlankTagName, DelimiterInTagName, TagNameTaken>;
		Util::Either<RenameError, Util::Void> RenameTag (const QModelIndex&, const QString&);
	private:
		void MigrateStoredNames ();
		tag_id InsertTag (const QString&);
	signals:
		void tagsUpdated (const QStringList&);
	};
}
