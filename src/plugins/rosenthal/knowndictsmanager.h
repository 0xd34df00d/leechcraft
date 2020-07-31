/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QStringList>
#include <QMap>

class QAbstractItemModel;
class QStandardItemModel;
class QStringListModel;
class QStandardItem;

namespace LC
{
namespace Rosenthal
{
	class KnownDictsManager : public QObject
	{
		Q_OBJECT

		const QString LocalPath_;

		QStandardItemModel * const Model_;
		QStringList Languages_;
		QMap<QString, QString> Lang2Path_;

		QStringListModel * const EnabledModel_;
	public:
		KnownDictsManager ();

		QAbstractItemModel* GetModel () const;
		QStringList GetLanguages () const;
		QString GetDictPath (const QString& language) const;

		QAbstractItemModel* GetEnabledModel () const;
	private:
		void LoadSettings ();
		void SaveSettings ();
	private slots:
		void rebuildDictsModel ();

		void handleItemChanged (QStandardItem*);
		void reemitLanguages ();
	signals:
		void languagesChanged (const QStringList&);
	};
}
}
