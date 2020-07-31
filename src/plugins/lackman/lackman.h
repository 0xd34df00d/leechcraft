/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWidget>
#include <QTranslator>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include <interfaces/iinfo.h>
#include <interfaces/ihavesettings.h>
#include <interfaces/ihavetabs.h>
#include <interfaces/ihavesettings.h>
#include <interfaces/ientityhandler.h>
#include <interfaces/ihaveshortcuts.h>
#include <interfaces/ihaverecoverabletabs.h>

class QSortFilterProxyModel;
class QStringListModel;

namespace LC
{
namespace Util
{
	class ShortcutManager;
}
namespace LackMan
{
	class LackManTab;

	class Plugin : public QObject
				 , public IInfo
				 , public IHaveTabs
				 , public IHaveSettings
				 , public IEntityHandler
				 , public IHaveShortcuts
				 , public IHaveRecoverableTabs
	{
		Q_OBJECT
		Q_INTERFACES (IInfo
				IHaveTabs
				IHaveSettings
				IEntityHandler
				IHaveShortcuts
				IHaveRecoverableTabs)

		LC_PLUGIN_METADATA ("org.LeechCraft.LackMan")

		Util::XmlSettingsDialog_ptr SettingsDialog_;
		Util::ShortcutManager *ShortcutMgr_;

		TabClassInfo TabClass_;

		LackManTab *LackManTab_ = nullptr;
	public:
		void Init (ICoreProxy_ptr);
		void SecondInit ();
		void Release ();
		QByteArray GetUniqueID () const;
		QString GetName () const;
		QString GetInfo () const;
		QIcon GetIcon () const;

		TabClasses_t GetTabClasses () const;
		void TabOpenRequested (const QByteArray&);

		void TabOpenRequested (const QByteArray&, const QList<QPair<QByteArray, QVariant>>&);

		Util::XmlSettingsDialog_ptr GetSettingsDialog () const;

		EntityTestHandleResult CouldHandle (const Entity&) const;
		void Handle (Entity);

		void SetShortcut (const QString&, const QKeySequences_t&);
		QMap<QString, ActionInfo> GetActionInfo () const;

		void RecoverTabs (const QList<TabRecoverInfo>& infos);
		bool HasSimilarTab (const QByteArray&, const QList<QByteArray>&) const;
	signals:
		void gotEntity (const LC::Entity&);

		void addNewTab (const QString&, QWidget*);
		void removeTab (QWidget*);
		void changeTabName (QWidget*, const QString&);
		void changeTabIcon (QWidget*, const QIcon&);
		void changeTooltip (QWidget*, QWidget*);
		void statusBarChanged (QWidget*, const QString&);
		void raiseTab (QWidget*);
	};
}
}
