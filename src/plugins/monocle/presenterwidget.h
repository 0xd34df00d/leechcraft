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

namespace LC
{
namespace Monocle
{
	class PresenterWidget : public QWidget
	{
		Q_OBJECT

		QLabel *PixmapLabel_;
		IDocument_ptr Doc_;
		int CurrentPage_;
	public:
		PresenterWidget (IDocument_ptr);

		void NavigateTo (int);
	protected:
		void closeEvent (QCloseEvent*);
		void keyPressEvent (QKeyEvent*);
	private slots:
		void delayedShowInit ();
	};
}
}
