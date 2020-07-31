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
class QAbstractItemModel;
class QUrl;

namespace LC
{
namespace Scroblibre
{
	class AccountsManager : public QObject
	{
		Q_OBJECT

		QStandardItemModel * const Model_;
	public:
		AccountsManager (QObject* = 0);

		void LoadAccounts ();

		QAbstractItemModel* GetModel () const;
	private:
		void SaveSettings ();
		void AddRow (const QVariantList&);
	public slots:
		void addRequested (const QString&, const QVariantList&);
		void removeRequested (const QString&, const QModelIndexList&);
	signals:
		void accountAdded (const QUrl&, const QString&);
		void accountRemoved (const QUrl&, const QString&);
	};
}
}
