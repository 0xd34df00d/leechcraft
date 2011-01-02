/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
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

#ifndef TAGSVIEWER_H
#define TAGSVIEWER_H
#include <QWidget>
#include "ui_tagsviewer.h"

namespace LeechCraft
{
	class TagsViewer : public QWidget
	{
		Q_OBJECT

		Ui::TagsViewer Ui_;
	public:
		TagsViewer (QWidget* = 0);
	private slots:
		void on_Rename__released ();
		void on_Remove__released ();
	};
};

#endif

