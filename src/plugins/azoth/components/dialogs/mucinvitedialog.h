/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QDialog>
#include "ui_mucinvitedialog.h"

namespace LC
{
namespace Azoth
{
	class IAccount;

	class MUCInviteDialog : public QDialog
	{
		Q_OBJECT

		Ui::MUCInviteDialog Ui_;
		bool ManualMode_;
	public:
		enum class ListType
		{
			ListEntries,
			ListMucs
		};

		MUCInviteDialog (IAccount*, ListType = ListType::ListEntries, QWidget* = 0);

		QString GetID () const;
		void SetID (const QString&);

		QString GetInviteMessage () const;
	private slots:
		void on_Invitee__currentIndexChanged ();
		void on_Invitee__editTextChanged ();
	};
}
}
