/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWidget>
#include <interfaces/ihavetabs.h>
#include <interfaces/core/icoreproxy.h>

class QTermWidget;

namespace LC::Util
{
	class ShortcutManager;
}

namespace LC::Eleeminator
{
	class ColorSchemesManager;

	class TermTab : public QWidget
				  , public ITabWidget
	{
		Q_OBJECT
		Q_INTERFACES (ITabWidget)

		QObject * const ParentPlugin_;

		QToolBar * const Toolbar_;

		QTermWidget& Term_;

		bool IsTabCurrent_ = false;
	public:
		TermTab (Util::ShortcutManager*, const ColorSchemesManager&, QObject*);

		static TabClassInfo GetStaticTabClassInfo ();

		TabClassInfo GetTabClassInfo () const override;
		QObject* ParentMultiTabs () override;
		QToolBar* GetToolBar () const override;
		void Remove () override;
		void TabMadeCurrent () override;
		void TabLostCurrent () override;
	private:
		void SetupToolbar (Util::ShortcutManager*, const ColorSchemesManager&);
		void SetupShortcuts (Util::ShortcutManager*);

		void HandleBell (const QString&) const;

		void RemoveTab ();
	signals:
		void changeTabName (const QString&) override;
		void removeTab () override;
	};
}
