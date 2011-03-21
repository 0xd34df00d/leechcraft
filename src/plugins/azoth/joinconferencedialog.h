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

#ifndef PLUGINS_AZOTH_JOINCONFERENCEDIALOG_H
#define PLUGINS_AZOTH_JOINCONFERENCEDIALOG_H
#include <QDialog>
#include "interfaces/iaccount.h"
#include "ui_joinconferencedialog.h"

namespace LeechCraft
{
namespace Azoth
{
	class JoinConferenceDialog : public QDialog
	{
		Q_OBJECT

		Ui::JoinConferenceDialog Ui_;
		QHash<IProtocol*, QWidget*> Proto2Joiner_;
	public:
		JoinConferenceDialog (const QList<IAccount*>&, QWidget* = 0);
		virtual ~JoinConferenceDialog ();
	public slots:
		virtual void accept ();
		virtual void reject ();
	private slots:
		virtual void on_AccountBox__currentIndexChanged (int);
		virtual void on_BookmarksBox__currentIndexChanged (int);
		virtual void on_HistoryBox__currentIndexChanged (int);
	};
}
}

#endif
