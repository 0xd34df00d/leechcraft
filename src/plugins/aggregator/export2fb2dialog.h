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

#ifndef PLUGINS_AGGREGATOR_EXPORT2FB2DIALOG_H
#define PLUGINS_AGGREGATOR_EXPORT2FB2DIALOG_H
#include <QDialog>
#include "ui_export2fb2dialog.h"

namespace LeechCraft
{
	struct Entity;

	namespace Util
	{
		class CategorySelector;
	};

	namespace Plugins
	{
		namespace Aggregator
		{
			class Export2FB2Dialog : public QDialog
			{
				Q_OBJECT

				Ui::Export2FB2Dialog Ui_;
				Util::CategorySelector *Selector_;
				QStringList CurrentCategories_;
			public:
				Export2FB2Dialog (QWidget* = 0);
			private slots:
				void on_Browse__released ();
				void on_File__textChanged (const QString&);
				void handleChannelsSelectionChanged (const QItemSelection&, const QItemSelection&);
				void handleAccepted ();
			signals:
				void gotEntity (const LeechCraft::Entity&);
			};
		};
	};
};

#endif

