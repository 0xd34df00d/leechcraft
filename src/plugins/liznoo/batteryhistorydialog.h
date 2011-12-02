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

#ifndef PLUGINS_LIZNOO_BATTERYHISTORYDIALOG_H
#define PLUGINS_LIZNOO_BATTERYHISTORYDIALOG_H
#include <QObject>
#include <QLinkedList>
#include "ui_batteryhistorydialog.h"
#include "batteryhistory.h"

class QwtPlotCurve;

namespace LeechCraft
{
namespace Liznoo
{
	class BatteryHistoryDialog : public QDialog
	{
		Q_OBJECT

		Ui::BatteryHistoryDialog Ui_;
		
		QwtPlotCurve *Percent_;
		QwtPlotCurve *Energy_;
	public:
		BatteryHistoryDialog (int, QWidget* = 0);

		void UpdateHistory (const QLinkedList<BatteryHistory>&);
	};
}
}

#endif
