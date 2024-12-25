/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "tagsmanager.h"
#include <stdexcept>
#include <algorithm>
#include <QStringList>
#include <QCoreApplication>
#include <QSettings>
#include <QtDebug>
#include <util/sll/prelude.h>
#include <util/sll/qtutil.h>
#include <util/tags/tagscompleter.h>

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
	const auto& keys = Tags_.keys (tag);
	if (keys.isEmpty ())
		return InsertTag (tag);
	else if (keys.size () > 1)
		throw std::runtime_error (qPrintable (QString ("More than one key for %1").arg (tag)));
	else
		return keys.at (0).toString ();
}

QString TagsManager::GetTag (ITagsManager::tag_id id) const
{
	return Tags_ [id];
}

QStringList TagsManager::GetAllTags () const
{
	return Tags_.values ();
}

QStringList TagsManager::Split (const QString& string) const
{
	return Util::Map (string.split (";", Qt::SkipEmptyParts),
			[] (QString& s) { return std::move (s).trimmed (); });
}

QList<ITagsManager::tag_id> TagsManager::SplitToIDs (const QString& string)
{
	return GetIDs (Split (string));
}

QString TagsManager::Join (const QStringList& tags) const
{
	return tags.join ("; ");
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

void TagsManager::SetTag (const QModelIndex& index, const QString& newTag)
{
	if (!index.isValid ())
		return;

	auto pos = std::next (Tags_.begin (), index.row ());
	*pos = newTag;

	Storage_.SetTagName (pos.key (), newTag);

	emit dataChanged (index, index);

	emit tagsUpdated (GetAllTags ());
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

		for (const auto& [id, name] : Util::Stlize (tags))
			Storage_.AddTag (id, name);
		settings.setValue ("Migrated", true);
	}
	settings.endGroup ();
}
