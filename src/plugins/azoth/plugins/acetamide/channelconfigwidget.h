/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWidget>
#include <QDateTime>
#include <interfaces/azoth/iconfigurablemuc.h>
#include "ui_channelconfigwidget.h"
#include "localtypes.h"

class QStandardItemModel;

namespace LC::Azoth::Acetamide
{
	class ChannelCLEntry;
	class SortFilterProxyModel;

	class ChannelConfigWidget : public QWidget
								, public IMUCConfigWidget
	{
		Q_OBJECT
		Q_INTERFACES (LC::Azoth::IMUCConfigWidget)

		Ui::ChannelConfigWidget Ui_;
		ChannelCLEntry * const ChannelEntry_;
		ChannelModes ChannelMode_;
		QStandardItemModel * const BanModel_;
		QStandardItemModel * const ExceptModel_;
		QStandardItemModel * const InviteModel_;
		SortFilterProxyModel * const BanFilterModel_;
		SortFilterProxyModel * const ExceptFilterModel_;
		SortFilterProxyModel * const InviteFilterModel_;
	public:
		explicit ChannelConfigWidget (ChannelCLEntry*, QWidget* = nullptr);
	public slots:
		void accept () override;
	private:
		void SetupListButtons ();
		void RerequestList (int);

		void HandleNewChannelModes (const ChannelModes&);
	signals:
		void dataReady () override;
	};
}
