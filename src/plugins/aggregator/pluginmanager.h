/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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

#ifndef PLUGINS_AGGREGATOR_PLUGINMANAGER_H
#define PLUGINS_AGGREGATOR_PLUGINMANAGER_H
#include <QVariant>
#include <interfaces/core/ihookproxy.h>
#include <util/basehookinterconnector.h>
#include "interfaces/aggregator/item.h"
#include "proxyobject.h"

namespace LeechCraft
{
namespace Aggregator
{
	class PluginManager : public Util::BaseHookInterconnector
	{
		Q_OBJECT

		boost::shared_ptr<ProxyObject> ProxyObject_;
	public:
		PluginManager (QObject* = 0);

		virtual void AddPlugin (QObject*);
	signals:
		void hookItemLoad (LeechCraft::IHookProxy_ptr proxy,
				Item*);
		void hookGotNewItems (LeechCraft::IHookProxy_ptr proxy,
				QVariantList items);
	};
}
}

#endif
