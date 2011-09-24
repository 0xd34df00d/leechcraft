/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#ifndef PLUGINS_POSHUKU_PLUGINS_ONLINEBOOKMARKS_PLUGINS_READITLATER_READITLATERAUTHWIDGET_H
#define PLUGINS_POSHUKU_PLUGINS_ONLINEBOOKMARKS_PLUGINS_READITLATER_READITLATERAUTHWIDGET_H

#include <QWidget>
#include <interfaces/iauthwidget.h>
#include "ui_readitlaterauthwidget.h"

namespace LeechCraft
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
		Q_INTERFACES (LeechCraft::Poshuku::OnlineBookmarks::IAuthWidget)

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
