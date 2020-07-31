/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "quarkmanager.h"
#include <QtDebug>
#include <QApplication>
#include <util/gui/util.h>
#include <util/gui/autoresizemixin.h>
#include "maillistview.h"

namespace LC
{
namespace GmailNotifier
{
	QuarkManager::QuarkManager (ICoreProxy_ptr proxy, QObject *parent)
	: QObject (parent)
	, Proxy_ (proxy)
	{
	}

	int QuarkManager::GetMsgCount () const
	{
		return Infos_.size ();
	}

	void QuarkManager::handleConversations (const ConvInfos_t& infos)
	{
		const auto oldCount = Infos_.size ();
		Infos_ = infos;

		if (oldCount != infos.size ())
			emit msgCountChanged ();
	}

	void QuarkManager::showMailList (int x, int y, const QRect& geometry)
	{
		if (MailListView_)
		{
			delete MailListView_;
			return;
		}

		MailListView_ = new MailListView (Infos_, Proxy_);
		new Util::AutoResizeMixin ({ x, y }, [geometry] { return geometry; }, MailListView_);
		MailListView_->show ();
	}
}
}
