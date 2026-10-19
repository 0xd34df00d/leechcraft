/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "tagsmanager.h"
#include <algorithm>
#include <QStringList>
#include <QCoreApplication>
#include <QSettings>
#include <QtDebug>
#include <util/sll/prelude.h>
#include <util/sll/qtutil.h>
#include <util/tags/tagscompleter.h>
#include <util/tags/util.h>

using namespace LC;

TagsManager::TagsManager ()
{
	MigrateToDb ();

	for (const auto& [id, name] : Storage_.GetAllTags ())
		Tags_ [id] = name;

	GetID (tr ("untagged"));

	Util::TagsCompleter::SetModel (GetModel ());
}

TagsManager& TagsManager::Instance ()
{
	static TagsManager tm;
	return tm;
}

int TagsManager::columnCount (const QModelIndex&) const
{
	return 1;
}

QVariant TagsManager::data (const QModelIndex& index, int role) const
{
	if (!index.isValid () ||
			role != Qt::DisplayRole)
		return QVariant ();

	TagsDictionary_t::const_iterator pos = Tags_.begin ();
	std::advance (pos, index.row ());
	return *pos;
}

QModelIndex TagsManager::index (int row, int column, const QModelIndex& parent) const
{
	if (!hasIndex (row, column, parent))
		return QModelIndex ();

	return createIndex (row, column);
}

QModelIndex TagsManager::parent (const QModelIndex&) const
{
	return QModelIndex ();
}

int TagsManager::rowCount (const QModelIndex& index) const
{
	return index.isValid () ? 0 : Tags_.size ();
}

ITagsManager::tag_id TagsManager::GetID (const QString& tag)
{
	const auto& name = tag.trimmed ();
	if (name.isEmpty () || name.contains (Util::GetTagsDelimiter (Util::GetDefaultTagsSeparator ())))
	{
		qWarning () << "invalid tag name" << tag;
		return {};
	}

	const auto& ids = Tags_.keys (name);
	if (ids.isEmpty ())
		return InsertTag (name);

	if (ids.size () > 1)
		qWarning () << ids.size () << "tags named" << name;
	return ids.front ().toString ();
}

QString TagsManager::GetTag (ITagsManager::tag_id id) const
{
	return Tags_ [QUuid { id }];
}

QStringList TagsManager::GetAllTags () const
{
	return Tags_.values ();
}

QStringList TagsManager::Split (const QString& string) const
{
	return Util::SplitTags (string, Util::GetDefaultTagsSeparator ());
}

QList<ITagsManager::tag_id> TagsManager::SplitToIDs (const QString& string)
{
	return GetIDs (Split (string));
}

QString TagsManager::Join (const QStringList& tags) const
{
	return tags.join (Util::GetDefaultTagsSeparator ());
}

QString TagsManager::JoinIDs (const QStringList& tagIDs) const
{
	const auto& hr = Util::Map (tagIDs,
			[this] (const QString& id) { return GetTag (id); });
	return Join (hr);
}

ITagsManager::tag_id TagsManager::InsertTag (const QString& tag)
{
	const auto& uuid = QUuid::createUuid ();

	Storage_.AddTag (uuid, tag);

	auto updated = Tags_;
	auto pos = updated.insert (uuid, tag);
	const auto dist = std::distance (updated.begin (), pos);

	beginInsertRows ({}, dist, dist);
	Tags_ = std::move (updated);
	endInsertRows ();

	emit tagsUpdated (GetAllTags ());

	return uuid.toString ();
}

void TagsManager::RemoveTag (const QModelIndex& index)
{
	if (!index.isValid ())
		return;

	const auto pos = std::next (Tags_.begin (), index.row ());

	Storage_.DeleteTag (pos.key ());

	beginRemoveRows (QModelIndex (), index.row (), index.row ());
	Tags_.erase (pos);
	endRemoveRows ();

	emit tagsUpdated (GetAllTags ());
}

Util::Either<TagsManager::RenameError, Util::Void> TagsManager::RenameTag (const QModelIndex& index, const QString& newName)
{
	if (!index.isValid ())
		return Util::Void {};

	if (const auto& delimiter = Util::GetTagsDelimiter (Util::GetDefaultTagsSeparator ()); newName.contains (delimiter))
		return Util::Left { DelimiterInTagName { delimiter } };

	const auto& name = newName.trimmed ();
	if (name.isEmpty ())
		return Util::Left { BlankTagName {} };

	const auto pos = std::next (Tags_.begin (), index.row ());
	if (*pos == name)
		return Util::Void {};

	if (!Tags_.key (name).isNull ())
		return Util::Left { TagNameTaken { name } };

	*pos = name;
	Storage_.SetTagName (pos.key (), name);

	emit dataChanged (index, index);
	emit tagsUpdated (GetAllTags ());

	return Util::Void {};
}

QAbstractItemModel* TagsManager::GetModel ()
{
	return this;
}

QObject* TagsManager::GetQObject ()
{
	return this;
}

// TODO post 0.6.75 drop this
void TagsManager::MigrateToDb ()
{
	QSettings settings (QCoreApplication::organizationName (),
			QCoreApplication::applicationName ());
	settings.beginGroup ("Tags");
	if (!settings.value ("Migrated").toBool ())
	{
		const auto& tags = settings.value ("Dict").value<TagsDictionary_t> ();

		qDebug () << Q_FUNC_INFO << "migrating" << tags.size () << "tags to the DB";

		for (const auto& [id, name] : tags.asKeyValueRange ())
			Storage_.AddTag (id, name);
		settings.setValue ("Migrated", true);
	}
	settings.endGroup ();
}
