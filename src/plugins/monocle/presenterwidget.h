/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWidget>
#include "interfaces/monocle/idocument.h"

class QLabel;

namespace LC::Monocle
{
	class PresenterWidget : public QWidget
	{
		Q_OBJECT

		QLabel& PixmapLabel_;
		IDocument_ptr Doc_;
		int CurrentPage_ = 0;
	public:
		explicit PresenterWidget (IDocument_ptr);

		void NavigateTo (int);
	protected:
		void closeEvent (QCloseEvent*) override;
		void keyPressEvent (QKeyEvent*) override;
	private slots:
		void delayedShowInit ();
	};
}
