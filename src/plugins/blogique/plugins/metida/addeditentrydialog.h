/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
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

#pragma once

#include <QDialog>
#include "ui_addeditentrydialog.h"

namespace LeechCraft
{
namespace Blogique
{
namespace Metida
{
	class LJProfile;

	enum AddTypeEntry
	{
		ATEFriend,
		ATEGroup
	};

	class AddEditEntryDialog : public QDialog
	{
		Q_OBJECT

		Ui::AddEditEntryDialog Ui_;

		enum GroupRoles
		{
			RealGroupId = Qt::UserRole + 1
		};

		enum AddTypeRoles
		{
			AddType = Qt::UserRole + 2
		};
		

		LJProfile *Profile_;
		QColor BackgroundColor_;
		QColor ForegroundColor_;
	public:
		AddEditEntryDialog (LJProfile *profile, QWidget *parent = 0);

		QString GetUserName () const;
		void SetUserName (const QString& name);
		QString GetBackgroundColorName () const;
		void SetBackgroundColor (const QColor& clr);
		QString GetForegroundColorName () const;
		void SetForegroundColor (const QColor& clr);
		uint GetGroupRealId () const;
		void SetGroup (uint id);

		QString GetGroupName () const;
		void SetGroupName (const QString& name);
		bool GetAcccess () const;
		void SetAccess (bool isPublic);

		AddTypeEntry GetAddTypeEntry () const;
		void ShowAddTypePossibility (bool show);
		void SetCurrentAddTypeEntry (AddTypeEntry entry);

		void accept ();
	private slots:
		void on_SelectBackgroundColor__released ();
		void on_SelectForegroundColor__released ();
	};
}
}
}
