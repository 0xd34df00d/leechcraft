/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QHash>
#include <QModelIndex>
#include "interfaces/azoth/itransfermanager.h"

class QStandardItemModel;
class QStandardItem;
class QAbstractItemModel;
class QToolBar;
class QUrl;

namespace LC
{
namespace Azoth
{
	class ICLEntry;
	class AvatarsManager;

	class TransferJobManager : public QObject
	{
		Q_OBJECT

		AvatarsManager * const AvatarsMgr_;

		QStandardItemModel * const SummaryModel_;

		enum ModelRoles
		{
			MRJobObject = Qt::UserRole + 1
		};

		typedef QHash<QObject*, QStandardItem*> ObjectDictionary_t;
		ObjectDictionary_t Object2Status_;
		ObjectDictionary_t Object2Progress_;

		QHash<QString, QObjectList> Entry2Incoming_;
		QHash<ITransferJob*, QString> Job2SavePath_;

		QPersistentModelIndex Selected_;
		QToolBar * const ReprBar_;
	public:
		TransferJobManager (AvatarsManager*, QObject* = nullptr);

		void AddAccountManager (QObject*);
		QObjectList GetPendingIncomingJobsFor (const QString&);

		void SelectionChanged (const QModelIndex&);

		void HandleJob (QObject*);
		void AcceptJob (QObject*, QString);
		void DenyJob (QObject*);
		QAbstractItemModel* GetSummaryModel () const;

		bool OfferURLs (ICLEntry *entry, QList<QUrl> urls);
	private:
		QString CheckSavePath (QString);
		void HandleDeoffer (QObject*);

		void HandleTaskFinished (ITransferJob*);
	private slots:
		void handleFileOffered (QObject*);
		void handleXferError (TransferError, const QString&);
		void handleStateChanged (TransferState);
		void handleXferProgress (qint64, qint64);
		void handleAbortAction ();
	signals:
		void jobNoLongerOffered (QObject*);
	};
}
}
