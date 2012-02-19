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

#include "webaccess.h"
#include <QIcon>
#include <interfaces/aggregator/iproxyobject.h>
#include "servermanager.h"

namespace LeechCraft
{
namespace Aggregator
{
namespace WebAccess
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Proxy_ = proxy;
	}

	void Plugin::SecondInit ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Aggregator.WebAccess";
	}

	void Plugin::Release ()
	{
		SM_.reset ();
	}

	QString Plugin::GetName () const
	{
		return "Aggregator WebAccess";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Provides remote HTTP/Web access to Aggregator.");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon ();
	}

	QSet<QByteArray> Plugin::GetPluginClasses () const
	{
		QSet<QByteArray> result;
		result << "org.LeechCraft.Aggregator.GeneralPlugin/1.0";
		return result;
	}

	void Plugin::initPlugin (QObject *proxy)
	{
		try
		{
			SM_.reset (new ServerManager (qobject_cast<IProxyObject*> (proxy), Proxy_));
		}
		catch (const std::exception& e)
		{
			qWarning () << Q_FUNC_INFO
					<< e.what ();
		}
	}
}
}
}

LC_EXPORT_PLUGIN (leechcraft_aggregator_webaccess, LeechCraft::Aggregator::WebAccess::Plugin);
