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

#ifndef PLUGINS_LCFTP_SUMMARYTAB_H
#define PLUGINS_LCFTP_SUMMARYTAB_H
#include <QWidget>
#include "ui_summarytab.h"

class QTreeView;

namespace LeechCraft
{
	namespace Plugins
	{
		namespace LCFTP
		{
			class SummaryTab : public QWidget
			{
				Q_OBJECT

				Ui::SummaryTab Ui_;
				QModelIndex Current_;
			public:
				SummaryTab (QWidget* = 0);
			public slots:
				void handleCurrentChanged (const QModelIndex&);
				void updateTab ();
				void on_DownloadSpeed__valueChanged (int);
				void on_UploadSpeed__valueChanged (int);
			};
		};
	};
};

#endif

