/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_POSHUKU_PLUGINS_ONLINEBOOKMARKS_PLUGINS_READITLATER_READITLATERAUTHWIDGET_H
#define PLUGINS_POSHUKU_PLUGINS_ONLINEBOOKMARKS_PLUGINS_READITLATER_READITLATERAUTHWIDGET_H

#include <QWidget>
#include <interfaces/iauthwidget.h>
#include "ui_readitlaterauthwidget.h"

namespace LC
{
namespace Poshuku
{
namespace OnlineBookmarks
{
namespace ReadItLater
{
	class ReadItLaterAuthWidget : public QWidget
								, public IAuthWidget
	{
		Q_OBJECT
		Q_INTERFACES (LC::Poshuku::OnlineBookmarks::IAuthWidget)

		Ui::AuthWidget Ui_;
	public:
		ReadItLaterAuthWidget (QWidget* = 0);
		QVariantMap GetIdentifyingData () const;
		void SetIdentifyingData (const QVariantMap&);
	};
}
}
}
}

#endif // PLUGINS_POSHUKU_PLUGINS_ONLINEBOOKMARKS_PLUGINS_READITLATER_READITLATERAUTHWIDGET_H
