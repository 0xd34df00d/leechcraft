/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <functional>
#include <QObject>
#include <QMap>

class QModelIndex;
class QAbstractItemModel;
class QStandardItemModel;
class QStandardItem;

namespace LC
{
namespace Azoth
{
class ICLEntry;

namespace Xoox
{
	class GlooxAccount;

	class Xep0313ModelManager : public QObject
	{
		Q_OBJECT

		QStandardItemModel * const Model_;
		QMap<QString, QStandardItem*> Jid2Item_;
	public:
		Xep0313ModelManager (GlooxAccount*);

		QAbstractItemModel* GetModel () const;
		QString Index2Jid (const QModelIndex&) const;
		QModelIndex Jid2Index (const QString&) const;
	private:
		void PerformWithEntries (const QList<QObject*>&, const std::function<void (ICLEntry*)>&);
	private slots:
		void handleGotCLItems (const QList<QObject*>&);
		void handleRemovedCLItems (const QList<QObject*>&);
	};
}
}
}
