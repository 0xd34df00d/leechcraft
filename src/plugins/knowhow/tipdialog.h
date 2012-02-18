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

#ifndef PLUGINS_KNOWHOW_TIPDIALOG_H
#define PLUGINS_KNOWHOW_TIPDIALOG_H
#include <memory>
#include <QDialog>
#include <interfaces/core/icoreproxy.h>
#include "ui_tipdialog.h"

class QDomDocument;

namespace LeechCraft
{
namespace KnowHow
{
	class TipDialog : public QDialog
	{
		Q_OBJECT

		Ui::TipDialog Ui_;
		std::shared_ptr<QDomDocument> Doc_;
		ICoreProxy_ptr Proxy_;
	public:
		TipDialog (ICoreProxy_ptr, QWidget* = 0);
	private:
		void ShowForIdx (int);
		QString GetTipByID (int);
	private slots:
		void on_Forward__released ();
		void on_Backward__released ();
		void on_DontShow__stateChanged ();
	};
}
}

#endif
