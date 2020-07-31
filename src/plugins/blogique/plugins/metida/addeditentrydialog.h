/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QDialog>
#include "ui_addeditentrydialog.h"

namespace LC
{
namespace Blogique
{
namespace Metida
{
	class LJProfile;

	enum AddTypeEntry
	{
		ATENone,
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
		uint GroupMask_;

	public:
		AddEditEntryDialog (LJProfile *profile, AddTypeEntry type = ATENone, QWidget *parent = 0);

		QString GetUserName () const;
		void SetUserName (const QString& name);
		QString GetBackgroundColorName () const;
		void SetBackgroundColor (const QColor& clr);
		QString GetForegroundColorName () const;
		void SetForegroundColor (const QColor& clr);
		uint GetGroupMask () const;
		void SetGroupMask (uint mask);

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
		void on_SelectGroups__released ();
	};
}
}
}
