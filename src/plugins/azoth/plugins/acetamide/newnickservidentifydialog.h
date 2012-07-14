/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
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

#ifndef PLUGINS_AZOTH_PLUGINS_ACETAMIDE_NEWNICKSERVIDENTIFYDIALOG_H
#define PLUGINS_AZOTH_PLUGINS_ACETAMIDE_NEWNICKSERVIDENTIFYDIALOG_H

#include <QDialog>
#include "ui_newnickservidentifydialog.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Acetamide
{
	class NewNickServIdentifyDialog : public QDialog
	{
		Q_OBJECT

		Ui::NewNickServDataDialog Ui_;
	public:
		NewNickServIdentifyDialog (QWidget* = 0, Qt::WindowFlags = 0);
		QString GetServer () const;
		QString GetNickName () const;
		QString GetNickServNickName () const;
		QString GetAuthString () const;
		QString GetAuthMessage () const;
		void SetServer (const QString&);
		void SetNickName (const QString&);
		void SetNickServNickName (const QString&);
		void SetAuthString (const QString&);
		void SetAuthMessage (const QString&);
	public slots:
		void accept ();
	};
}
}
}

#endif // PLUGINS_AZOTH_PLUGINS_ACETAMIDE_NEWNICKSERVIDENTIFYDIALOG_H