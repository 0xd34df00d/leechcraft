/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QTextEdit>

namespace LC
{
namespace Azoth
{
	class TextEdit : public QTextEdit
	{
		Q_OBJECT

		QFont DefaultFont_;
	public:
		TextEdit (QWidget *parent = 0);
	protected:
		void keyPressEvent (QKeyEvent*);
	private slots:
		void handleMsgFontSize ();

		void deleteWord ();
		void deleteBOL ();
		void deleteEOL ();
	signals:
		void keyReturnPressed ();
		void keyTabPressed ();
		void clearAvailableNicks ();
		void scroll (int);
	};
}
}
