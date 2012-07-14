/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
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

#ifndef PLUGINS_AZOTH_PLUGINS_HERBICIDE_CONFWIDGET_H
#define PLUGINS_AZOTH_PLUGINS_HERBICIDE_CONFWIDGET_H
#include <QWidget>
#include "ui_confwidget.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Herbicide
{
	class ConfWidget : public QWidget
	{
		Q_OBJECT
		
		Ui::ConfWidget Ui_;
		QList<QList<QPair<QString, QStringList>>> PredefinedQuests_;
	public:
		ConfWidget (QWidget* = 0);
		
		QString GetQuestion () const;
		QStringList GetAnswers () const;
	private:
		void SaveSettings () const;
		void LoadSettings ();
	public slots:
		void accept ();
		void reject ();
	private slots:
		void on_QuestStyle__currentIndexChanged (int);
		void on_QuestVariant__currentIndexChanged (int);
	};
}
}
}

#endif
