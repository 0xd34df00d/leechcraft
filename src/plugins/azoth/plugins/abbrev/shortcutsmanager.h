/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QShortcut>
#include <QMap>
#include <QCoreApplication>
#include <interfaces/ihaveshortcuts.h>

namespace LC::Azoth::Abbrev
{
	class AbbrevsManager;

	class ShortcutsManager : public QObject
	{
		Q_DECLARE_TR_FUNCTIONS (LC::Azoth::Abbrev::ShortcutsManager)

		AbbrevsManager * const Abbrevs_;

		QMap<QWidget*, QShortcut*> Tab2SC_;

		QKeySequence Sequence_;
	public:
		explicit ShortcutsManager (AbbrevsManager*, QObject* = nullptr);

		void HandleTab (QWidget*);

		QMap<QString, ActionInfo> GetActionInfo () const;
		void SetShortcut (const QString&, const QKeySequences_t&);
	private:
		void HandleActivated (QWidget*);
	};
}
