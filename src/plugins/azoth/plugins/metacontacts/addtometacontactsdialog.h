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

#ifndef PLUGINS_AZOTH_PLUGINS_METACONTACTS_ADDTOMETACONTACTSDIALOG_H
#define PLUGINS_AZOTH_PLUGINS_METACONTACTS_ADDTOMETACONTACTSDIALOG_H
#include <QDialog>
#include "ui_addtometacontactsdialog.h"

namespace LeechCraft
{
namespace Azoth
{
class ICLEntry;

namespace Metacontacts
{
	class MetaEntry;

	class AddToMetacontactsDialog : public QDialog
	{
		Q_OBJECT
		
		Ui::AddToMetacontactsDialog Ui_;
	public:
		AddToMetacontactsDialog (ICLEntry*,
				const QList<MetaEntry*>&, QWidget* = 0);
		
		MetaEntry* GetSelectedMeta () const;
		QString GetNewMetaName () const;
	private slots:
		void on_ExistingMeta__currentIndexChanged (int);
	};
}
}
}

#endif
