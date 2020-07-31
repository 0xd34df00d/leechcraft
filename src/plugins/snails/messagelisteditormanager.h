/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QModelIndex>

class QTreeView;

namespace LC
{
namespace Snails
{
	enum class MailListMode;

	class MessageListEditorManager : public QObject
	{
		QTreeView * const View_;
		QModelIndex LastEdited_;
		MailListMode Mode_;
	public:
		MessageListEditorManager (QTreeView*, QObject* = nullptr);

		void SetMailListMode (MailListMode);
		void HandleMessageListUpdated ();
	private:
		void CloseCurrent ();
	};
}
}
