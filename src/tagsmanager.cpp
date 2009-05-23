#include "tagsmanager.h"
#include <QSettings>
#include <QtDebug>
#include <plugininterface/proxy.h>

using namespace LeechCraft;

TagsManager::TagsManager ()
{
	GetID (tr ("untagged"));
	ReadSettings ();
}

TagsManager& TagsManager::Instance ()
{
	static TagsManager tm;
	return tm;
}

TagsManager::~TagsManager ()
{
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
	QList<QUuid> keys = Tags_.keys (tag);
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

QStringList TagsManager::Split (const QString& string) const
{
	return string.split (";", QString::SkipEmptyParts);
}

QString TagsManager::Join (const QStringList& tags) const
{
	return tags.join ("; ");
}

ITagsManager::tag_id TagsManager::InsertTag (const QString& tag)
{
	beginInsertRows (QModelIndex (), Tags_.size (), Tags_.size ());
	QUuid uuid = QUuid::createUuid ();
	Tags_ [uuid] = tag;
	endInsertRows ();
	WriteSettings ();
	return uuid.toString ();
}

void TagsManager::RemoveTag (const QModelIndex& index)
{
	if (!index.isValid ())
		return;

	TagsDictionary_t::iterator pos = Tags_.begin ();
	std::advance (pos, index.row ());
	beginRemoveRows (QModelIndex (), index.row (), index.row ());
	Tags_.erase (pos);
	endRemoveRows ();
	WriteSettings ();
}

void TagsManager::SetTag (const QModelIndex& index, const QString& newTag)
{
	if (!index.isValid ())
		return;

	TagsDictionary_t::iterator pos = Tags_.begin ();
	std::advance (pos, index.row ());
	*pos = newTag;

	emit dataChanged (index, index);
}

QAbstractItemModel* TagsManager::GetModel ()
{
	return this;
}

void TagsManager::ReadSettings ()
{
	QSettings settings (Util::Proxy::Instance ()->GetOrganizationName (),
			Util::Proxy::Instance ()->GetApplicationName ());
	settings.beginGroup ("Tags");
	Tags_ = settings.value ("Dict").value<TagsDictionary_t> ();
	beginInsertRows (QModelIndex (), 0, Tags_.size () - 1);
	endInsertRows ();
	settings.endGroup ();
}

void TagsManager::WriteSettings () const
{
	QSettings settings (Util::Proxy::Instance ()->GetOrganizationName (),
			Util::Proxy::Instance ()->GetApplicationName ());
	settings.beginGroup ("Tags");
	settings.setValue ("Dict", QVariant::fromValue<TagsDictionary_t> (Tags_));
	settings.endGroup ();
}

