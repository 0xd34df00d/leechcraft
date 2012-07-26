/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
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

#pragma once

#include <QObject>
#include <QVariantList>
#include <QModelIndexList>

class QStandardItemModel;
class QAbstractItemModel;

namespace LeechCraft
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
