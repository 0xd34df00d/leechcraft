/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QModelIndexList>

class QStandardItemModel;

namespace LC
{
namespace Monocle
{
	class DefaultBackendManager : public QObject
	{
		Q_OBJECT

		QStandardItemModel *Model_;
		enum Roles
		{
			KeyID = Qt::UserRole + 1
		};
	public:
		DefaultBackendManager (QObject* = 0);

		void LoadSettings ();

		QAbstractItemModel* GetModel () const;
		QObject* GetBackend (const QList<QObject*>&);
	private:
		void AddToModel (const QByteArray&, const QByteArray&);
	public slots:
		void removeRequested (const QString&, const QModelIndexList&);
	};
}
}
