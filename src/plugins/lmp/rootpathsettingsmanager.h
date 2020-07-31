/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QModelIndex>
#include <QVariant>

class QStandardItemModel;
class QAbstractItemModel;

namespace LC
{
namespace LMP
{
	class RootPathSettingsManager : public QObject
	{
		Q_OBJECT

		QStandardItemModel *Model_;
	public:
		RootPathSettingsManager (QObject* = 0);

		QAbstractItemModel* GetModel () const;
	public slots:
		void addRequested (const QString&, const QVariantList&);
		void removeRequested (const QString&, const QModelIndexList&);
	private slots:
		void handleRootPathsChanged ();
	};
}
}
