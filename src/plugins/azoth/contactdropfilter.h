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

namespace LC
{
namespace Azoth
{
	class ChatTab;

	class ContactDropFilter : public QObject
	{
		Q_OBJECT

		const QString EntryId_;
		ChatTab * const ChatTab_;
	public:
		ContactDropFilter (const QString&, ChatTab*);

		bool eventFilter (QObject*, QEvent*);

		void HandleDrop (const QMimeData*);
	private:
		bool CheckImage (const QList<QUrl>&);

		void CollectDataFilters (QStringList& choiceItems,
				QList<std::function<void ()>>& functions,
				const QImage& image);

		void HandleImageDropped (const QImage&, const QUrl&);
		void PerformChoice (const QStringList&, const QList<std::function<void ()>>&);

		void HandleContactsDropped (const QMimeData*);
		void HandleFilesDropped (const QList<QUrl>&);
	};
}
}
