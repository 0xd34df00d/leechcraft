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

#ifndef PLUGINS_KNOWHOW_TIPDIALOG_H
#define PLUGINS_KNOWHOW_TIPDIALOG_H
#include <boost/shared_ptr.hpp>
#include <QDialog>
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
		boost::shared_ptr<QDomDocument> Doc_;
	public:
		TipDialog (QWidget* = 0);
	private:
		QString GetTipByID (int);
		void ShowTip (const QString&);
	};
}
}

#endif
