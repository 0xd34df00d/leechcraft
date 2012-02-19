/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2012  Georg Rudoy
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

#include "gacts.h"
#include <QIcon>
#include <QxtGui/QxtGlobalShortcut>
#include <interfaces/entitytesthandleresult.h>

namespace LeechCraft
{
namespace GActs
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
	}

	void Plugin::SecondInit ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.GActs";
	}

	void Plugin::Release ()
	{
		qDeleteAll (RegisteredShortcuts_.values ());
		RegisteredShortcuts_.clear ();
	}

	QString Plugin::GetName () const
	{
		return "GActs";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Provides support for Global Actions registration for other LeechCraft plugins.");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon ();
	}

	EntityTestHandleResult Plugin::CouldHandle (const Entity& e) const
	{
		const bool good = (e.Mime_ == "x-leechcraft/global-action-register" ||
					e.Mime_ == "x-leechcraft/global-action-unregister") &&
				e.Additional_.contains ("ActionID");
		return EntityTestHandleResult (good ?
					EntityTestHandleResult::PIdeal :
					EntityTestHandleResult::PNone);
	}

	void Plugin::Handle (Entity e)
	{
		const QByteArray& id = e.Additional_ ["ActionID"].toByteArray ();

		if (e.Mime_ == "x-leechcraft/global-action-unregister")
		{
			auto sh = RegisteredShortcuts_.take (id);
			if (sh)
				delete sh;
			return;
		}

		QObject *receiver = e.Additional_ ["Receiver"].value<QObject*> ();
		if (!receiver)
			return;

		const QByteArray& method = e.Additional_ ["Method"].toByteArray ();
		if (method.isEmpty ())
			return;

		const QKeySequence& seq = e.Additional_ ["Shortcut"].value<QKeySequence> ();

		connect (receiver,
				SIGNAL (destroyed (QObject*)),
				this,
				SLOT (handleReceiverDeleted ()),
				Qt::UniqueConnection);

		QxtGlobalShortcut *sh = new QxtGlobalShortcut (seq, receiver);
		connect (sh,
				SIGNAL (activated ()),
				receiver,
				method);
		RegisteredShortcuts_ [id] = sh;
	}

	void Plugin::handleReceiverDeleted ()
	{
		Q_FOREACH (auto sh, RegisteredShortcuts_.values ())
			if (sh->parent () == sender ())
				RegisteredShortcuts_.remove (RegisteredShortcuts_.key (sh));
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_gacts, LeechCraft::GActs::Plugin);
