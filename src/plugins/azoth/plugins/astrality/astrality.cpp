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
#include <TelepathyQt/Types>
#include <TelepathyQt/Debug>
#include <TelepathyQt/ConnectionManager>
#include <TelepathyQt/PendingStringList>
#include <util/util.h>
#include "cmwrapper.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Astrality
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
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
		qDeleteAll (Wrappers_);
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Azoth.Astrality";
	}

	QString Plugin::GetName () const
	{
		return "Astrality";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Support for protocols provided by Telepathy.");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon ();
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
		return QList<QObject*> ();
	}

	void Plugin::initPlugin (QObject *proxy)
	{
	}

	void Plugin::handleListNames (Tp::PendingOperation *op)
	{
		auto psl = qobject_cast<Tp::PendingStringList*> (op);
		qDebug () << Q_FUNC_INFO << psl->result ();

		Q_FOREACH (const QString& cmName, psl->result ())
			Wrappers_ << new CMWrapper (cmName, this);
	}
}
}
}

Q_EXPORT_PLUGIN2 (leechcraft_azoth_astrality, LeechCraft::Azoth::Astrality::Plugin);
