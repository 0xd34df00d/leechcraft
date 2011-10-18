/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2011 Minh Ngo
 * Copyright (C) 2006-2011  Georg Rudoy
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

#ifndef PLUGINS_LAURE_PLAYLISTWIDGET_H
#define PLUGINS_LAURE_PLAYLISTWIDGET_H

#include <QWidget>
#include <interfaces/core/icoreproxy.h>
#include "ui_playlistwidget.h"

class QToolBar;

namespace LeechCraft
{
namespace Laure
{
	class PlayListView;
	
	class PlayListWidget : public QWidget
	{
		Q_OBJECT
		
		Ui::PlayListWidget Ui_;
		QToolBar *ActionBar_;
	public:
		PlayListWidget (QWidget* = 0);
		PlayListView* GetPlayListView () const;
	signals:
		void itemPlayed (int);
	};
}
}

#endif // PLUGINS_LAURE_PLAYLISTWIDGET_H
