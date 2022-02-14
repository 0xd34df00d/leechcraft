/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "shortcutmanager.h"
#include <QAction>
#include <QShortcut>
#include <interfaces/entityconstants.h>
#include <util/xpc/util.h>
#include <util/sll/prelude.h>
#include "interfaces/ihaveshortcuts.h"
#include "interfaces/core/ientitymanager.h"
#include "interfaces/core/iiconthememanager.h"
#include "interfaces/core/ishortcutproxy.h"

namespace LC::Util
{
	ShortcutManager::ShortcutManager (const ICoreProxy_ptr& proxy, QObject *parent)
	: QObject { parent }
	, CoreProxy_ { proxy }
	{
	}

	void ShortcutManager::SetObject (QObject *obj)
	{
		ContextObj_ = obj;
	}

	void ShortcutManager::RegisterAction (const QString& id, QAction *act)
	{
		Actions_ [id] << act;
		connect (act,
				&QObject::destroyed,
				this,
				[this, act]
				{
					for (auto& list : Actions_)
						list.removeAll (act);
				});

		if (HasActionInfo (id))
		{
			const auto& info = ActionInfo_ [id];
			if (act->text ().isEmpty ())
				act->setText (info.UserVisibleText_);
			if (act->icon ().isNull () &&
					act->property ("ActionIcon").isNull ())
				act->setIcon (info.Icon_);
		}
		else
		{
			const auto& icon = act->icon ().isNull () ?
					CoreProxy_->GetIconThemeManager ()->GetIcon (act->property ("ActionIcon").toString ()) :
					act->icon ();
			RegisterActionInfo (id,
					{
						act->text (),
						act->shortcuts (),
						icon
					});
		}

		if (CoreProxy_->GetShortcutProxy ()->HasObject (ContextObj_))
			SetShortcut (id,
					CoreProxy_->GetShortcutProxy ()->GetShortcuts (ContextObj_, id));
	}

	void ShortcutManager::RegisterActions (const std::initializer_list<IDPair_t>& pairs)
	{
		for (const auto& [id, act] : pairs)
			RegisterAction (id, act);
	}

	void ShortcutManager::RegisterShortcut (const QString& id, const ActionInfo& info, QShortcut *shortcut)
	{
		Shortcuts_ [id] << shortcut;
		connect (shortcut,
				&QObject::destroyed,
				this,
				[this, shortcut]
				{
					for (auto& list : Shortcuts_)
						list.removeAll (shortcut);

					qDeleteAll (Shortcut2Subs_.take (shortcut));
				});

		RegisterActionInfo (id, info);

		if (CoreProxy_->GetShortcutProxy ()->HasObject (ContextObj_))
			SetShortcut (id,
					CoreProxy_->GetShortcutProxy ()->GetShortcuts (ContextObj_, id));
	}

	void ShortcutManager::RegisterActionInfo (const QString& id, const ActionInfo& info)
	{
		if (!HasActionInfo (id))
			ActionInfo_ [id] = info;
	}

	void ShortcutManager::RegisterGlobalShortcut (const QString& id,
			QObject *target, const QByteArray& method, const ActionInfo& info)
	{
		Entity e = Util::MakeEntity ({}, {}, {}, Mimes::GlobalActionRegister);
		using namespace EF::GlobalAction;
		e.Additional_ [Receiver] = QVariant::fromValue (target);
		e.Additional_ [ActionID] = id;
		e.Additional_ [Method] = method;
		e.Additional_ [Shortcut] = QVariant::fromValue (info.Seqs_.value (0));
		e.Additional_ [AltShortcuts] = Util::Map (info.Seqs_.mid (1), &QVariant::fromValue<QKeySequence>);
		Globals_ [id] = e;

		ActionInfo_ [id] = info;
	}

	void ShortcutManager::AnnounceGlobalShorcuts ()
	{
		for (const auto& entity : Globals_)
			CoreProxy_->GetEntityManager ()->HandleEntity (entity);
	}

	void ShortcutManager::SetShortcut (const QString& id, const QKeySequences_t& seqs)
	{
		for (auto act : Actions_ [id])
			act->setShortcuts (seqs);

		for (auto sc : Shortcuts_ [id])
		{
			sc->setKey (seqs.value (0));
			qDeleteAll (Shortcut2Subs_.take (sc));

			const int seqsSize = seqs.size ();
			for (int i = 1; i < seqsSize; ++i)
			{
				auto subsc = new QShortcut { sc->parentWidget () };
				subsc->setContext (sc->context ());
				subsc->setKey (seqs.value (i));
				connect (subsc,
						&QShortcut::activated,
						sc,
						&QShortcut::activated);
				Shortcut2Subs_ [sc] << subsc;
			}
		}

		if (Globals_.contains (id))
		{
			auto& e = Globals_ [id];
			e.Additional_ [QStringLiteral ("Shortcut")] = QVariant::fromValue (seqs.value (0));
			e.Additional_ [QStringLiteral ("AltShortcuts")] = Util::Map (seqs.mid (1),
					&QVariant::fromValue<QKeySequence>);
			CoreProxy_->GetEntityManager ()->HandleEntity (e);
		}
	}

	QMap<QString, ActionInfo> ShortcutManager::GetActionInfo () const
	{
		return ActionInfo_;
	}

	ShortcutManager& ShortcutManager::operator<< (const QPair<QString, QAction*>& pair)
	{
		RegisterAction (pair.first, pair.second);
		return *this;
	}

	bool ShortcutManager::HasActionInfo (const QString& id) const
	{
		return ActionInfo_.contains (id) &&
				!ActionInfo_ [id].UserVisibleText_.isEmpty ();
	}
}
