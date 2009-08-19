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

#ifndef PLUGINS_CSTP_ADDTASK_H
#define PLUGINS_CSTP_ADDTASK_H
#include <QDialog>
#include <QUrl>
#include "ui_addtask.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace CSTP
		{
			class AddTask : public QDialog
			{
				Q_OBJECT

				Ui::AddTask Ui_;
				bool UserModifiedFilename_;
			public:
				AddTask (QWidget* = 0);
				AddTask (const QUrl&, const QString&, QWidget* = 0);
				virtual ~AddTask ();

				struct Task
				{
					QUrl URL_;
					QString LocalPath_;
					QString Filename_;
					QString Comment_;

					Task (const QUrl&,
							const QString&,
							const QString&,
							const QString&);
				};

				Task GetTask () const;
			public slots:
				virtual void accept ();
			private slots:
				void on_URL__textEdited (const QString&);
				void on_LocalPath__textChanged ();
				void on_Filename__textEdited ();
				void on_BrowseButton__released ();
			private:
				void CheckOK ();
			};
		};
	};
};

#endif

