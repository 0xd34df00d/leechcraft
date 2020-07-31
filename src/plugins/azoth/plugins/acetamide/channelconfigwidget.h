/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/


#ifndef PLUGINS_AZOTH_PLUGINS_ACETAMIDE_CHANNELCONFIGWIDGET_H
#define PLUGINS_AZOTH_PLUGINS_ACETAMIDE_CHANNELCONFIGWIDGET_H

#include <QWidget>
#include <QDateTime>
#include <interfaces/azoth/iconfigurablemuc.h>
#include "ui_channelconfigwidget.h"
#include "localtypes.h"

class QStandardItemModel;

namespace LC
{
namespace Azoth
{
namespace Acetamide
{

	class ChannelCLEntry;
	class SortFilterProxyModel;

	class ChannelConfigWidget : public QWidget
								, public IMUCConfigWidget
	{
		Q_OBJECT
		Q_INTERFACES (LC::Azoth::IMUCConfigWidget)

		Ui::ChannelConfigWidget Ui_;
		ChannelCLEntry *ChannelEntry_;
		ChannelModes ChannelMode_;
		QStandardItemModel *BanModel_;
		QStandardItemModel *ExceptModel_;
		QStandardItemModel *InviteModel_;
		SortFilterProxyModel *BanFilterModel_;
		SortFilterProxyModel *ExceptFilterModel_;
		SortFilterProxyModel *InviteFilterModel_;
		bool IsWidgetRequest_;
	public:
		ChannelConfigWidget (ChannelCLEntry*, QWidget* = 0);
	private:
		void SetModesUi ();
	public slots:
		void accept ();
		void on_BanSearch__textChanged (const QString&);
		void on_ExceptSearch__textChanged (const QString&);
		void on_InviteSearch__textChanged (const QString&);
		void on_tabWidget_currentChanged (int);
		void on_UpdateBan__clicked ();
		void on_AddBan__clicked ();
		void on_RemoveBan__clicked ();
		void on_UpdateExcept__clicked ();
		void on_AddExcept__clicked ();
		void on_RemoveExcept__clicked ();
		void on_AddInvite__clicked ();
		void on_UpdateInvite__clicked ();
		void on_RemoveInvite__clicked ();

		void addBanListItem (const QString&, 
				const QString&, const QDateTime&);
		void addExceptListItem (const QString&, 
				const QString&, const QDateTime&);
		void addInviteListItem (const QString&, 
				const QString&, const QDateTime&);

		void handleNewChannelModes (const ChannelModes&);
	signals:
		void dataReady ();
	};
}
}
}
#endif // PLUGINS_AZOTH_PLUGINS_ACETAMIDE_CHANNELCONFIGWIDGET_H
