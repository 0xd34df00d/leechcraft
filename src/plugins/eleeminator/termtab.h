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

class QMenu;
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

		const TabClassInfo TC_;
		QObject * const ParentPlugin_;

		QToolBar * const Toolbar_;

		QTermWidget * const Term_;

		ColorSchemesManager * const ColorSchemesMgr_;
		QString CurrentColorScheme_;

		bool IsTabCurrent_ = false;
	public:
		TermTab (Util::ShortcutManager*, const TabClassInfo&, ColorSchemesManager*, QObject*);

		TabClassInfo GetTabClassInfo () const;
		QObject* ParentMultiTabs ();
		QToolBar* GetToolBar () const;
		void Remove ();
		void TabMadeCurrent ();
		void TabLostCurrent ();
	private:
		void SetupToolbar (Util::ShortcutManager*);
		void SetupColorsButton ();
		void SetupFontsButton ();

		void SetupShortcuts (Util::ShortcutManager*);

		void AddUrlActions (QMenu&, const QPoint&);
		void AddLocalFileActions (QMenu&, const QString&);
	private slots:
		void setHistorySettings ();

		void handleTermContextMenu (const QPoint&);

		void openUrl ();
		void copyUrl ();

		void setColorScheme (QAction*);
		void previewColorScheme (QAction*);
		void stopColorSchemePreview ();

		void selectFont ();

		void updateTitle ();

		void handleUrlActivated (const QUrl&);
		void handleBell (const QString&);

		void handleFinished ();
	signals:
		void changeTabName (const QString&);
		void removeTab ();
	};
}
