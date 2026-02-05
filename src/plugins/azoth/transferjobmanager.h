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

		QHash<QString, QList<IncomingOffer>> Entry2Incoming_;

		QPersistentModelIndex Selected_;
		QToolBar * const ReprBar_;
	public:
		TransferJobManager (AvatarsManager*, QObject* = nullptr);

		void AddAccountManager (QObject*);

		void SelectionChanged (const QModelIndex&);
		QAbstractItemModel* GetSummaryModel () const;

		void AcceptOffer (const IncomingOffer&, QString);
		void DeclineOffer (const IncomingOffer&);
		QList<IncomingOffer> GetIncomingOffers (const QString&);

		struct OutgoingFileOffer
		{
			ICLEntry& Entry_;
			QString Variant_;
			QString FilePath_;
			QString Comment_;
		};
		bool SendFile (const OutgoingFileOffer&);

		struct JobContext
		{
			struct In { QString SavePath_; };
			struct Out {};
			std::variant<In, Out> Dir_;

			QString OrigFilename_;
			qint64 Size_;

			QString EntryName_;
			QString EntryId_;
		};
	private:
		void HandleJob (ITransferJob*, const JobContext&);

		void Deoffer (const IncomingOffer&);
		void NotifyDeoffer (const IncomingOffer&);

		void HandleIncomingFinished (const JobContext&, const JobContext::In&);
		void HandleFileOffered (const IncomingOffer&);
		void HandleStateChanged (TransferState, const JobContext&, QStandardItem*);
	private slots:
		void handleAbortAction ();
	signals:
		void jobNoLongerOffered (const IncomingOffer&);
	};
}
}
