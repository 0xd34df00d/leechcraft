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

#ifndef PLUGINS_POSHUKU_HISTORYWIDGET_H
#define PLUGINS_POSHUKU_HISTORYWIDGET_H
#include <QWidget>
#include <plugininterface/tagscompleter.h>
#include "ui_historywidget.h"
#include "historyfiltermodel.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Poshuku
		{
			class HistoryWidget : public QWidget
			{
				Q_OBJECT

				Ui::HistoryWidget Ui_;
				std::auto_ptr<HistoryFilterModel> HistoryFilterModel_;
			public:
				HistoryWidget (QWidget* = 0);
			private slots:
				void on_HistoryView__activated (const QModelIndex&);
				void updateHistoryFilter ();
			};
		};
	};
};

#endif

