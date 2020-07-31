/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QVariantList>
#include <QModelIndexList>

class QStandardItemModel;
class QAbstractItemModel;

namespace LC
{
namespace LMP
{
namespace MP3Tunes
{
	class AccountsManager : public QObject
	{
		Q_OBJECT

		QStandardItemModel *AccModel_;
	public:
		AccountsManager (QObject* = 0);

		QAbstractItemModel* GetAccModel () const;
		QStringList GetAccounts () const;
	private:
		void SaveAccounts ();
		void LoadAccounts ();
	public slots:
		void addRequested (const QString&, const QVariantList&);
		void removeRequested (const QString&, const QModelIndexList&);
	signals:
		void accountsChanged ();
	};
}
}
}
