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

#ifndef PLUGINS_POSHUKU_PLUGINS_POSHUKU_PASSWORDREMEMBER_H
#define PLUGINS_POSHUKU_PLUGINS_POSHUKU_PASSWORDREMEMBER_H
#include "notification.h"
#include "pageformsdata.h"
#include "ui_passwordremember.h"

namespace LeechCraft
{
	struct Entity;

	namespace Plugins
	{
		namespace Poshuku
		{
			class PasswordRemember : public Notification
			{
				Q_OBJECT

				Ui::PasswordRemember Ui_;
				PageFormsData_t TempData_;
			public:
				PasswordRemember (QWidget* = 0);
			public slots:
				void add (const PageFormsData_t&);
			private:
				bool Changed (const ElementsData_t&, const QString&);
			private slots:
				void on_Remember__released ();
				void on_NotNow__released ();
				void on_Never__released ();
			signals:
				void delegateEntity (const LeechCraft::Entity&, int*, QObject**);
			};
		};
	};
};

#endif

