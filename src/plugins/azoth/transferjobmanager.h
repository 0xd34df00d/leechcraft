/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010  Georg Rudoy
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

#ifndef PLUGINS_AZOTH_TRANSFERJOBMANAGER_H
#define PLUGINS_AZOTH_TRANSFERJOBMANAGER_H
#include "interfaces/itransfermanager.h"
#include <QObject>
#include <QHash>

class QStandardItemModel;
class QStandardItem;
class QAbstractItemModel;

namespace LeechCraft
{
namespace Azoth
{
	class TransferJobManager : public QObject
	{
		Q_OBJECT

		QStandardItemModel *SummaryModel_;

		enum ModelRoles
		{
			MRJobObject = Qt::UserRole + 1
		};

		typedef QHash<QObject*, QStandardItem*> ObjectDictionary_t;
		ObjectDictionary_t Object2Status_;
		ObjectDictionary_t Object2Progress_;
	public:
		TransferJobManager (QObject* = 0);

		void AddAccountManager (QObject*);

		void HandleJob (QObject*);
		void AcceptJob (QObject*, QString);
		void DenyJob (QObject*);
		QAbstractItemModel* GetSummaryModel () const;
	private:
		QString CheckSavePath (QString);
	private slots:
		void handleFileOffered (QObject*);
		void handleXferError (TransferError, const QString&);
		void handleStateChanged (TransferState);
		void handleXferProgress (qint64, qint64);
	};
}
}

#endif
