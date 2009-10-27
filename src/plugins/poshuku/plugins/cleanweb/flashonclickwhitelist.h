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

#ifndef PLUGINS_POSHUKU_PLUGINS_CLEANWEB_FLASHONCLICKWHITELIST_H
#define PLUGINS_POSHUKU_PLUGINS_CLEANWEB_FLASHONCLICKWHITELIST_H
#include <QWidget>
#include <QStringList>
#include "ui_flashonclickwhitelist.h"

class QStandardItemModel;

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
					class FlashOnClickWhitelist : public QWidget
					{
						Q_OBJECT

						Ui::FlashOnClickWhitelist Ui_;
						QStandardItemModel *Model_;
					public:
						FlashOnClickWhitelist (QWidget* = 0);

						QStringList GetWhitelist () const;
						bool Matches (const QString&) const;
						void Add (const QString&);
					private slots:
						void on_Add__released ();
						void on_Modify__released ();
						void on_Remove__released ();
					private:
						void AddImpl (QString = QString (), const QModelIndex& = QModelIndex ());
						void SaveSettings ();
					};
				};
			};
		};
	};
};

#endif

