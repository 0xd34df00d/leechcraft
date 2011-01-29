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

#ifndef PLUGINS_POSHUKU_PLUGINS_CLEANWEB_SUBSCRIPTIONSMANAGER_H
#define PLUGINS_POSHUKU_PLUGINS_CLEANWEB_SUBSCRIPTIONSMANAGER_H
#include <QWidget>
#include "ui_subscriptionsmanager.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Poshuku
		{
			namespace Plugins
			{
				namespace CleanWeb
				{
					class SubscriptionsManager : public QWidget
					{
						Q_OBJECT

						Ui::SubscriptionsManager Ui_;
					public:
						SubscriptionsManager (QWidget* = 0);
					private:
						void AddCustom (const QString&, const QString&);
					private slots:
						void on_AddButton__released ();
						void on_RemoveButton__released ();
					};
				};
			};
		};
	};
};

#endif

