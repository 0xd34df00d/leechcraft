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


#ifndef PLUGINS_AZOTH_PLUGINS_ACETAMIDE_CHANNELCONFIGWIDGET_H
#define PLUGINS_AZOTH_PLUGINS_ACETAMIDE_CHANNELCONFIGWIDGET_H

#include <QWidget>
#include <QDateTime>
#include <interfaces/azoth/iconfigurablemuc.h>
#include "ui_channelconfigwidget.h"
#include "localtypes.h"

class QStandardItemModel;

namespace LeechCraft
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
		Q_INTERFACES (LeechCraft::Azoth::IMUCConfigWidget)

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
