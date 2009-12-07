/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#ifndef TAGSMANAGER_H
#define TAGSMANAGER_H
#include "interfaces/iinfo.h"
#include <QAbstractItemModel>
#include <QMap>
#include <QUuid>
#include <QString>
#include <QMetaType>

namespace LeechCraft
{
	class TagsManager : public QAbstractItemModel
					  , public ITagsManager
	{
		Q_OBJECT
		Q_INTERFACES (ITagsManager);

		TagsManager ();
	public:
		typedef QMap<QUuid, QString> TagsDictionary_t;
	private:
		TagsDictionary_t Tags_;
	public:
		static TagsManager& Instance ();
		virtual ~TagsManager ();

		int columnCount (const QModelIndex&) const;
		QVariant data (const QModelIndex&, int) const;
		QModelIndex index (int, int, const QModelIndex&) const;
		QModelIndex parent (const QModelIndex&) const;
		int rowCount (const QModelIndex&) const;

		tag_id GetID (const QString&);
		QString GetTag (tag_id) const;
		QStringList GetAllTags () const;
		QStringList Split (const QString&) const;
		QString Join (const QStringList&) const;
		QAbstractItemModel* GetModel ();
		QObject* GetObject ();

		void RemoveTag (const QModelIndex&);
		void SetTag (const QModelIndex&, const QString&);
	private:
		tag_id InsertTag (const QString&);
		void ReadSettings ();
		void WriteSettings () const;
	signals:
		void tagsUpdated (const QStringList&);
	};
};

Q_DECLARE_METATYPE (LeechCraft::TagsManager::TagsDictionary_t);

#endif

