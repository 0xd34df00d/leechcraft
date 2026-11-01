/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "tagsmanager.h"
#include <algorithm>
#include <QDir>
#include <QStringList>
#include <QtDebug>
#include <util/sll/qtutil.h>
#include <util/sys/paths.h>
#include <util/tags/util.h>

using namespace LC;

namespace
{
	std::optional<TagsManager::InvalidTagName> Validate (const QString& name)
	{
		if (const auto& delimiter = Util::GetTagsDelimiter (Util::GetDefaultTagsSeparator ());
			name.contains (delimiter))
			return TagsManager::DelimiterInTagName { delimiter };
		if (name.isEmpty ())
			return TagsManager::BlankTagName {};
		return {};
	}

	QStringList DistinctNames (const QHash<QUuid, std::flat_set<QString>>& names)
	{
		std::flat_set<QString> all;
		for (const auto& own : names)
			all.insert (own.begin (), own.end ());
		return QStringList (all.begin (), all.end ());
	}
}

TagsManager::TagsManager (std::unique_ptr<TagsStorage> storage)
: Storage_ { std::move (storage) }
{
	for (const auto& [id, name] : Storage_->GetAllTags ())
		Names_ [id].insert (name);
	Rows_ = DistinctNames (Names_);
	MigrateStoredNames ();

	qRegisterMetaType<QList<TagId>> ();
}

TagsManager& TagsManager::Instance ()
{
	static TagsManager tm { std::make_unique<TagsStorage> (Util::GetUserDir (Util::UserDir::LC, "core").filePath ("core.db")) };
	return tm;
}

QVariant TagsManager::data (const QModelIndex& index, int role) const
{
	if (!index.isValid () ||
			(role != Qt::DisplayRole && role != Qt::EditRole))
		return {};

	return Rows_.value (index.row ());
}

int TagsManager::rowCount (const QModelIndex& index) const
{
	return index.isValid () ? 0 : Rows_.size ();
}

std::optional<TagId> TagsManager::GetID (const QString& tag)
{
	const auto& name = tag.trimmed ();
	if (Validate (name))
	{
		qWarning () << "invalid tag name" << tag;
		return {};
	}

	std::optional<QUuid> pure;
	for (const auto& [id, names] : Names_.asKeyValueRange ())
		if (names.size () == 1 && *names.begin () == name && (!pure || id < *pure))
			pure = id;

	if (pure)
		return TagId { pure->toString () };
	return InsertTag (name);
}

QStringList TagsManager::GetTags (const TagId& id) const
{
	const auto pos = Names_.find (QUuid { id.Id_ });
	if (pos == Names_.end ())
		return {};
	return QStringList (pos->begin (), pos->end ());
}

QStringList TagsManager::GetAllTags () const
{
	return Rows_;
}

QStringList TagsManager::Split (const QString& string) const
{
	return Util::SplitTags (string, Util::GetDefaultTagsSeparator ());
}

QList<TagId> TagsManager::SplitToIDs (const QString& string)
{
	return GetIDs (Split (string));
}

QString TagsManager::Join (const QStringList& tags) const
{
	return tags.join (Util::GetDefaultTagsSeparator ());
}

QString TagsManager::JoinIDs (const QList<TagId>& tagIDs) const
{
	return Join (GetTags (tagIDs));
}

TagId TagsManager::InsertTag (const QString& name)
{
	const auto& uuid = QUuid::createUuid ();

	Storage_->AddTag (uuid, name);
	Names_ [uuid] = { name };
	SetRows (DistinctNames (Names_));

	return TagId { uuid.toString () };
}

Util::Either<TagsManager::InvalidTagName, Util::Void> TagsManager::ReplaceName (const QString& old, const QStringList& newNames)
{
	std::flat_set<QString> names;
	for (const auto& newName : newNames)
	{
		const auto& name = newName.trimmed ();
		if (const auto& error = Validate (name))
			return Util::Left { *error };
		names.insert (name);
	}

	auto updated = Names_;
	bool changed = false;
	for (auto pos = updated.begin (); pos != updated.end (); )
	{
		auto& own = *pos;
		if (!own.contains (old))
		{
			++pos;
			continue;
		}

		if (!names.contains (old))
		{
			own.erase (old);
			changed = true;
		}
		for (const auto& name : names)
			changed |= own.insert (name).second;

		pos = own.empty () ? updated.erase (pos) : std::next (pos);
	}

	if (!changed)
		return Util::Void {};

	const auto& replacement = QStringList (names.begin (), names.end ());
	Storage_->ReplaceName (old, replacement);
	Names_ = std::move (updated);
	SetRows (DistinctNames (Names_));

	emit Emitter_.nameReplaced (old, replacement);

	return Util::Void {};
}

void TagsManager::SetRows (const QStringList& rows)
{
	for (qsizetype i = Rows_.size () - 1; i >= 0; --i)
		if (!std::ranges::binary_search (rows, Rows_ [i]))
		{
			beginRemoveRows ({}, i, i);
			Rows_.removeAt (i);
			endRemoveRows ();
		}

	for (qsizetype i = 0; i < rows.size (); ++i)
		if (i == Rows_.size () || Rows_ [i] != rows [i])
		{
			beginInsertRows ({}, i, i);
			Rows_.insert (i, rows [i]);
			endInsertRows ();
		}
}

QAbstractItemModel* TagsManager::GetModel ()
{
	return this;
}

// TODO drop this Jan 2027
void TagsManager::MigrateStoredNames ()
{
	const auto& delimiter = Util::GetTagsDelimiter (Util::GetDefaultTagsSeparator ());
	for (const auto& name : GetAllTags ())
	{
		const auto& repaired = auto { name }.replace (delimiter, " "_qs).trimmed ();
		if (repaired == name)
			continue;

		if (repaired.isEmpty ())
		{
			qWarning () << "cannot repair tag" << name << "to" << repaired;
			continue;
		}

		qDebug () << "repairing tag" << name << "to" << repaired;
		ReplaceName (name, { repaired });
	}
}
