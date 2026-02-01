/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QTextEdit>

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
	public:
		explicit TextEdit (QWidget *parent = nullptr);

		void SetShortcutManager (Util::ShortcutManager& mgr);
		void ForceUpdateVisibleLines ();
	protected:
		void keyPressEvent (QKeyEvent*) override;
	private:
		bool IsMessageSend (const QKeyEvent&) const;
		void UpdateVisibleLines ();
	signals:
		void messageSendRequested ();
		void scrollRequested (int);
	};
}
