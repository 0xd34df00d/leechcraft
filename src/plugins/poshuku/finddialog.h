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

#ifndef PLUGINS_POSHUKU_FINDDIALOG_H
#define PLUGINS_POSHUKU_FINDDIALOG_H
#include <qwebpage.h>
#include "notification.h"
#include "ui_finddialog.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Poshuku
		{
			class FindDialog : public Notification
			{
				Q_OBJECT

				Ui::FindDialog Ui_;
			public:
				FindDialog (QWidget* = 0);
				virtual ~FindDialog ();

				void SetText (const QString&);
				void SetSuccessful (bool);
				void Focus ();
			private slots:
				void on_Pattern__textChanged (const QString&);
				void on_FindButton__released ();
				void reject ();
			signals:
				void next (const QString&, QWebPage::FindFlags);
			};
		};
	};
};

#endif

