/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
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

#include "quarkmanager.h"
#include <QtDebug>
#include <QApplication>
#include <util/gui/util.h>
#include "maillistview.h"

namespace LeechCraft
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
		qApp->processEvents ();
		MailListView_->move (Util::FitRect ({ x, y }, MailListView_->size (), geometry,
				Util::FitFlag::NoOverlap));
		MailListView_->show ();
	}
}
}
