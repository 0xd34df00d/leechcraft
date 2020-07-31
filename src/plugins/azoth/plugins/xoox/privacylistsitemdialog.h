/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_AZOTH_PLUGINS_XOOX_PRIVACYLISTSITEMDIALOG_H
#define PLUGINS_AZOTH_PLUGINS_XOOX_PRIVACYLISTSITEMDIALOG_H
#include <QDialog>
#include "ui_privacylistsitemdialog.h"

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	class PrivacyListItem;

	class PrivacyListsItemDialog : public QDialog
	{
		Q_OBJECT
		
		Ui::PrivacyListsItemDialog Ui_;
		
		enum TypeNum
		{
			TNJID,
			TNSubscription,
			TNGroup
		};
		
		enum ActionNum
		{
			ANAllow,
			ANDeny
		};
	public:
		PrivacyListsItemDialog (QWidget* = 0);
		
		PrivacyListItem GetItem () const;
		void SetItem (const PrivacyListItem&);
	private slots:
		void on_Type__currentIndexChanged (int);
	};
}
}
}

#endif
