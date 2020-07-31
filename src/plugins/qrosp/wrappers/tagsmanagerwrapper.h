/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_QROSP_WRAPPERS_TAGSMANAGERWRAPPER_H
#define PLUGINS_QROSP_WRAPPERS_TAGSMANAGERWRAPPER_H
#include <QObject>
#include <QStringList>

class ITagsManager;
class QAbstractItemModel;

namespace LC
{
namespace Qrosp
{
	class TagsManagerWrapper : public QObject
	{
		Q_OBJECT

		ITagsManager *Manager_;
	public:
		TagsManagerWrapper (ITagsManager*);
	public slots:
		QString GetID (const QString& tag);
		QString GetTag (const QString& id) const;
		QStringList GetAllTags () const;
		QStringList Split (const QString& string) const;
		QString Join (const QStringList& tags) const;
		QAbstractItemModel* GetModel ();
		QObject* GetQObject ();
	signals:
		void tagsUpdated (const QStringList& tags);
	};
}
}

#endif
