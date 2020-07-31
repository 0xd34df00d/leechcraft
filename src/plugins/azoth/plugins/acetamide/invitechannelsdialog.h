/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011 Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_AZOTH_PLUGINS_ACETAMIDE_INVITECHANNELSDIALOG_H
#define PLUGINS_AZOTH_PLUGINS_ACETAMIDE_INVITECHANNELSDIALOG_H

#include <QDialog>
#include "ui_invitechannelsdialog.h"

class QStandardItemModel;

namespace LC
{
namespace Azoth
{
namespace Acetamide
{
	class InviteChannelsDialog : public QDialog
	{
		Q_OBJECT
		Ui::InviteChannelsDialog Ui_;
		QStandardItemModel *Model_;
		QStandardItemModel *ActionsModel_;
		enum ActionByDefault
		{
			Ask,
			JoinAll,
			IgnoreAll
		};

		enum ActionsRole
		{
			ActionRole = Qt::UserRole + 1
		};
	public:
		InviteChannelsDialog (const QString&,
				const QString&, QWidget* = 0);
		void AddInvitation (const QString&, const QString&);
		QStringList GetChannels () const;
	public slots:
		void accept ();
	};
};
};
};

#endif // PLUGINS_AZOTH_PLUGINS_ACETAMIDE_INVITECHANNELSDIALOG_H
