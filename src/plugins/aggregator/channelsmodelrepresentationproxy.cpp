/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "channelsmodelrepresentationproxy.h"
#include <QToolBar>
#include <QMenu>
#include <interfaces/structures.h>

namespace LC
{
namespace Aggregator
{
	QVariant ChannelsModelRepresentationProxy::data (const QModelIndex& index, int role) const
	{
		if (role == RoleControls)
			return QVariant::fromValue<QToolBar*> (Toolbar_);
		if (role == RoleAdditionalInfo)
			return QVariant::fromValue<QWidget*> (TabWidget_);
		if (role == RoleContextMenu)
			return QVariant::fromValue<QMenu*> (Menu_);

		return QIdentityProxyModel::data (index, role);
	}

	void ChannelsModelRepresentationProxy::SetWidgets (QToolBar *bar, QWidget *tab)
	{
		Toolbar_ = bar;
		TabWidget_ = tab;
	}

	void ChannelsModelRepresentationProxy::SetMenu (QMenu *menu)
	{
		Menu_ = menu;
	}
}
}
