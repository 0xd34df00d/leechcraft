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
#include <QModelIndex>
#include <QVariant>

class QStandardItemModel;
class QAbstractItemModel;

namespace LeechCraft
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
