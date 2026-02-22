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
#include "types.h"

class QAbstractItemModel;
class QToolBar;
class QUrl;

namespace LC::Util
{
	class ProgressManager;
	class ProgressModelRow;
}

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
		QHash<QString, QList<IncomingOffer>> Entry2Incoming_;
	public:
		explicit TransferJobManager (AvatarsManager*, QObject* = nullptr);

		void AddAccountManager (QObject*);

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
	private:
		void HandleJob (ITransferJob*, const Transfers::JobContext&);

		void Deoffer (const IncomingOffer&, Transfers::DeofferReason);
		void NotifyDeoffer (const IncomingOffer&, Transfers::DeofferReason);

		void HandleIncomingFinished (const Transfers::JobContext&, const Transfers::JobContext::In&);
		void HandleFileOffered (const IncomingOffer&);
		void HandleStateChanged (const TransferState&, const Transfers::JobContext&);
	signals:
		void jobOffered (const IncomingOffer&);
		void jobDeoffered (const IncomingOffer&, Transfers::DeofferReason);

		void jobInitialized (ITransferJob&, const Transfers::JobContext&);
	};
}
}
