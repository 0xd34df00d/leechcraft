/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QTextEdit>
#include "components/util/dnd.h"

namespace LC::Util
{
	class ShortcutManager;
}

namespace LC::Azoth
{
	class TextEdit : public QTextEdit
	{
		Q_OBJECT

		const QFont DefaultFont_;

		int PreviousHeight_ = 0;

		Qt::KeyboardModifiers SendMods_ {};
		bool AllowKeypadEnter_ = true;

		DndUtil::DropHandler DropHandler_;

		enum class DragOwner
		{
			Nobody,
			DropHandler,
			Editor,
		};
		DragOwner DragOwner_ = DragOwner::Nobody;
	public:
		explicit TextEdit (QWidget *parent = nullptr);

		void SetShortcutManager (Util::ShortcutManager& mgr);
		void SetDropHandler (DndUtil::DropHandler);
		void ForceUpdateVisibleLines ();
	protected:
		void keyPressEvent (QKeyEvent*) override;

		void dragEnterEvent (QDragEnterEvent*) override;
		void dragMoveEvent (QDragMoveEvent*) override;
		void dragLeaveEvent (QDragLeaveEvent*) override;
		void dropEvent (QDropEvent*) override;
	private:
		bool IsMessageSend (const QKeyEvent&) const;
		void UpdateVisibleLines ();
	signals:
		void messageSendRequested ();
		void scrollRequested (int);
	};
}
