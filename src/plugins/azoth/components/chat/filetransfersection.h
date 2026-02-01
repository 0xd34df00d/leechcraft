/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>

class QToolButton;

namespace LC::Azoth
{
	class ITransferJob;

	class ChatTab;
	class TransferJobManager;

	class FileTransferSection : public QObject
	{
		Q_OBJECT

		QToolButton& EventsButton_;
		ChatTab& Tab_;
		TransferJobManager& Transfers_;
	public:
		explicit FileTransferSection (QToolButton& eventsButton, ChatTab& tab, TransferJobManager&);
	private:
		void HandleFileNoLongerOffered (QObject*);
		void HandleOfferActionTriggered (ITransferJob *job);
	private slots:
		void handleFileOffered (QObject*);
	};
}
