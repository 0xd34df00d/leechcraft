/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2012  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "gacts.h"
#include <QIcon>
#include <util/util.h>
#include <qxtglobalshortcut.h>
#include <interfaces/entityconstants.h>
#include <interfaces/entitytesthandleresult.h>

namespace LC
{
namespace GActs
{
	void Plugin::Init (ICoreProxy_ptr)
	{
		Util::InstallTranslator ("gacts");
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
		const bool good = (e.Mime_ == Mimes::GlobalActionRegister ||
					e.Mime_ == Mimes::GlobalActionUnregister) &&
				e.Additional_.contains (GlobalAction::ActionID);
		return EntityTestHandleResult (good ?
					EntityTestHandleResult::PIdeal :
					EntityTestHandleResult::PNone);
	}

	void Plugin::Handle (Entity e)
	{
		const QByteArray& id = e.Additional_ [GlobalAction::ActionID].toByteArray ();

		RegisteredShortcuts_.remove (id);
		if (e.Mime_ == Mimes::GlobalActionUnregister)
			return;

		const QKeySequence& seq = e.Additional_ [GlobalAction::Shortcut].value<QKeySequence> ();
		if (seq.isEmpty ())
			return;

		QObject *receiver = e.Additional_ [GlobalAction::Receiver].value<QObject*> ();
		if (!receiver)
			return;

		const QByteArray& method = e.Additional_ [GlobalAction::Method].toByteArray ();
		if (method.isEmpty ())
			return;

		connect (receiver,
				SIGNAL (destroyed (QObject*)),
				this,
				SLOT (handleReceiverDeleted ()),
				Qt::UniqueConnection);

		const auto sh = std::make_shared<QxtGlobalShortcut> (seq, receiver);
		connect (sh.get (),
				SIGNAL (activated ()),
				receiver,
				method);
		RegisteredShortcuts_ [id] = sh;

		RegisterChildren (sh.get (), e);
	}

	void Plugin::RegisterChildren (QxtGlobalShortcut *sh, const Entity& e)
	{
		for (const auto& seqVar : e.Additional_ [GlobalAction::AltShortcuts].toList ())
		{
			const auto& subseq = seqVar.value<QKeySequence> ();
			if (subseq.isEmpty ())
				continue;

			const auto subsh = new QxtGlobalShortcut (subseq, sh);
			connect (subsh,
					SIGNAL (activated ()),
					sh,
					SIGNAL (activated ()));
		}
	}

	void Plugin::handleReceiverDeleted ()
	{
		for (auto i = RegisteredShortcuts_.begin (); i != RegisteredShortcuts_.end (); )
		{
			if ((*i)->parent () != sender ())
				++i;
			else
				i = RegisteredShortcuts_.erase (i);
		}
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_gacts, LC::GActs::Plugin);
