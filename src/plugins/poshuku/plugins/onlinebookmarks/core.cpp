/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010  Oleg Linkin
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

#include "core.h"
#include <QtDebug>
#include <QCoreApplication>
#include <QStandardItemModel>
#include <QSettings>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Poshuku
		{
			namespace Plugins
			{
				namespace OnlineBookmarks
				{
					Core::Core ()
					{
					}
					
					Core& Core::Instance ()
					{
						static Core c;
						return c;
					}
					
					void Core::SendEntity (const LeechCraft::Entity& e)
					{
						emit gotEntity (e);
					}
					
					QStandardItemModel* Core::CreateAccountModel ()
					{
						QStandardItemModel *model = new QStandardItemModel;
						QSettings settings (QCoreApplication::organizationName (),
								QCoreApplication::applicationName () + "_Poshuku_OnlineBookmarks");
						settings.beginGroup ("Accounts");
						
						Q_FOREACH (const QString& item, settings.childKeys ())
						{
							QList<QStandardItem*> itemList;
							
							Q_FOREACH (const QString& login, settings.value (item).toStringList ())
							{
								QStandardItem *loginItem = new QStandardItem (login);
								loginItem->setCheckable (true);
								itemList << loginItem;
							}
							QStandardItem *service = new QStandardItem (item);
							model->appendRow (service);
							service->appendRows (itemList);
						}
						settings.endGroup ();
						
						return model;
					}
				};
			};
		};
	};
};