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

#include "astrality.h"
#include <QIcon>
#include <QtDebug>
#include <Types>
#include <Debug>
#include <ConnectionManager>
#include <PendingStringList>
#include <util/util.h>
#include <interfaces/azoth/iprotocol.h>
#include <interfaces/azoth/iaccount.h>
#include "cmwrapper.h"
#include "accountwrapper.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Astrality
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Util::InstallTranslator ("azoth_astrality");
		Tp::registerTypes ();
		Tp::enableDebug (false);
		Tp::enableWarnings (false);
	}

	void Plugin::SecondInit ()
	{
		connect (Tp::ConnectionManager::listNames (),
				SIGNAL (finished (Tp::PendingOperation*)),
				this,
				SLOT (handleListNames (Tp::PendingOperation*)));
	}

	void Plugin::Release ()
	{
		Q_FOREACH (CMWrapper *cmWrapper, Wrappers_)
			Q_FOREACH (QObject *protocol, cmWrapper->GetProtocols ())
				Q_FOREACH (QObject *accObj,
						qobject_cast<IProtocol*> (protocol)->GetRegisteredAccounts ())
				{
					auto acc = qobject_cast<AccountWrapper*> (accObj);
					acc->Shutdown ();
				}

		qDeleteAll (Wrappers_);
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Azoth.Astrality";
	}

	QString Plugin::GetName () const
	{
		return "Azoth Astrality";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Support for protocols provided by Telepathy.");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon (":/azoth/astrality/resources/images/astrality.svg");
	}

	QSet<QByteArray> Plugin::GetPluginClasses () const
	{
		QSet<QByteArray> classes;
		classes << "org.LeechCraft.Plugins.Azoth.Plugins.IProtocolPlugin";
		return classes;
	}

	QObject* Plugin::GetObject ()
	{
		return this;
	}

	QList<QObject*> Plugin::GetProtocols () const
	{
		QList<QObject*> result;
		Q_FOREACH (auto cmw, Wrappers_)
			result << cmw->GetProtocols ();
		return result;
	}

	void Plugin::initPlugin (QObject *proxy)
	{
	}

	void Plugin::handleListNames (Tp::PendingOperation *op)
	{
		auto psl = qobject_cast<Tp::PendingStringList*> (op);
		qDebug () << Q_FUNC_INFO << psl->result ();

		Q_FOREACH (const QString& cmName, psl->result ())
		{
			auto cmw = new CMWrapper (cmName, this);
			Wrappers_ << cmw;

			connect (cmw,
					SIGNAL (gotProtoWrappers (QList<QObject*>)),
					this,
					SIGNAL (gotNewProtocols (QList<QObject*>)));
			connect (cmw,
					SIGNAL (gotProtoWrappers (QList<QObject*>)),
					this,
					SLOT (handleProtoWrappers (QList<QObject*>)));
		}
	}

	void Plugin::handleProtoWrappers (const QList<QObject*>& wrappers)
	{
		Q_FOREACH (QObject *obj, wrappers)
		{
			connect (obj,
					SIGNAL (gotEntity (LeechCraft::Entity)),
					this,
					SIGNAL (gotEntity (LeechCraft::Entity)));
			connect (obj,
					SIGNAL (delegateEntity (LeechCraft::Entity, int*, QObject**)),
					this,
					SIGNAL (delegateEntity (LeechCraft::Entity, int*, QObject**)));
		}
	}
}
}
}

LC_EXPORT_PLUGIN (leechcraft_azoth_astrality, LeechCraft::Azoth::Astrality::Plugin);
