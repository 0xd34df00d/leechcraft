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

class QMimeData;
class QImage;
class QUrl;

namespace LC::Azoth
{
	class ChatTab;
	class TransferJobManager;

	class ContactDropFilter : public QObject
	{
		Q_OBJECT

		TransferJobManager& Transfers_;
		ChatTab& ChatTab_;
	public:
		ContactDropFilter (TransferJobManager&, ChatTab&);

		bool eventFilter (QObject*, QEvent*) override;

		void HandleDrop (const QMimeData*);
	private:
		void HandleImageDropped (const QImage&, const QUrl&);
		void HandleContactsDropped (const QMimeData*);
	};
}
