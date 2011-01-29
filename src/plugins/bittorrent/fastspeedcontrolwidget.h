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

#ifndef PLUGINS_BITTORRENT_FASTSPEEDCONTROLWIDGET_H
#define PLUGINS_BITTORRENT_FASTSPEEDCONTROLWIDGET_H
#include <QWidget>
#include <QList>
#include "ui_fastspeedcontrolwidget.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace BitTorrent
		{
			class FastSpeedControlWidget : public QWidget
			{
				Q_OBJECT

				Ui::FastSpeedControlWidget Ui_;
				QList<QPair<QSpinBox*, QSpinBox*> > Widgets_;
			public:
				FastSpeedControlWidget (QWidget* = 0);
			private:
				void LoadSettings ();
				void SaveSettings ();
				void SetNum (int);
			private slots:
				void on_Box__valueChanged (int);
				void on_Slider__valueChanged (int);
			public slots:
				void accept ();
				void reject ();
			signals:
				void speedsChanged ();
			};
		};
	};
};

#endif

