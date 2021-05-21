/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "quarkproxy.h"
#include <QStandardItemModel>
#include "actionsmodel.h"
#include "enablesoundactionmanager.h"

namespace LC::AdvancedNotifications
{
	QuarkProxy::QuarkProxy (QObject *parent)
	: QObject (parent)
	, ActionsModel_ (new ActionsModel (this))
	{
		auto soundMgr = new EnableSoundActionManager (this);
		ActionsModel_->AddAction (soundMgr->GetAction ());
	}

	QVariant QuarkProxy::getActionsModel () const
	{
		return QVariant::fromValue<QObject*> (ActionsModel_);
	}
}
