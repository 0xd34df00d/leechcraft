/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
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

#ifndef PLUGINS_POSHUKU_SEARCHTEXT_H
#define PLUGINS_POSHUKU_SEARCHTEXT_H
#include <QDialog>
#include <interfaces/iinfo.h>
#include "ui_searchtext.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Poshuku
		{
			class SearchText : public QDialog
			{
				Q_OBJECT

				Ui::SearchText Ui_;
				QString Text_;
			public:
				SearchText (const QString&, QWidget* = 0);
			private slots:
				void doSearch ();
				void on_MarkAll__released ();
				void on_UnmarkAll__released ();
			signals:
				void gotEntity (const LeechCraft::DownloadEntity&);
			};
		};
	};
};

#endif

