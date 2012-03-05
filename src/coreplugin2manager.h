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

#ifndef COREPLUGIN2MANAGER_H
#define COREPLUGIN2MANAGER_H
#include <QNetworkAccessManager>
#include "util/basehookinterconnector.h"
#include "interfaces/core/ihookproxy.h"
#include "interfaces/iinfo.h"

class QMenu;

namespace LeechCraft
{
	class CorePlugin2Manager : public Util::BaseHookInterconnector
	{
		Q_OBJECT
	public:
		CorePlugin2Manager (QObject* = 0);
	signals:
		void hookGonnaFillQuickLaunch (LeechCraft::IHookProxy_ptr proxy);
		void hookNAMCreateRequest (LeechCraft::IHookProxy_ptr proxy,
					QNetworkAccessManager *manager,
					QNetworkAccessManager::Operation *op,
					QIODevice **dev);
		void hookTabContextMenuFill (LeechCraft::IHookProxy_ptr proxy,
					QMenu *menu, int index);
		void hookReleaseMouseAfterMove (LeechCraft::IHookProxy_ptr proxy,
				int index);
		void hookTabSetText (LeechCraft::IHookProxy_ptr proxy,
				int index);
	};
}

#endif
