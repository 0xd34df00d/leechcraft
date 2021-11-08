/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011 Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QCoreApplication>
#include <QDialog>
#include "ui_invitechannelsdialog.h"

class QStandardItemModel;

namespace LC::Azoth::Acetamide
{
	class InviteChannelsDialog : public QDialog
	{
		Q_DECLARE_TR_FUNCTIONS (LC::Azoth::Acetamide::InviteChannelsDialog)

		Ui::InviteChannelsDialog Ui_;
		QStandardItemModel * const Model_;
	public:
		InviteChannelsDialog (const QString&, const QString&, QWidget* = nullptr);

		void AddInvitation (const QString&, const QString&);
		QStringList GetChannels () const;
	protected:
		void accept () override;
	};
}
