/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QModelIndex>
#include <interfaces/iinfo.h>
#include <interfaces/ihavetabs.h>
#include <interfaces/ihavesettings.h>
#include <interfaces/ijobholder.h>
#include <interfaces/ihaveshortcuts.h>

namespace LC
{
namespace Util
{
	class WkFontsWidget;
	class ShortcutManager;
}

namespace Snails
{
	class AccountsManager;
	class ComposeMessageTabFactory;
	class MsgTemplatesManager;
	class ProgressManager;
	class Storage;

	class Plugin : public QObject
				 , public IInfo
				 , public IHaveTabs
				 , public IHaveSettings
				 , public IJobHolder
				 , public IHaveShortcuts
	{
		Q_OBJECT
		Q_INTERFACES (IInfo IHaveTabs IHaveSettings IJobHolder IHaveShortcuts)

		LC_PLUGIN_METADATA ("org.LeechCraft.Snails")

		TabClassInfo MailTabClass_;
		TabClassInfo ComposeTabClass_;

		Util::XmlSettingsDialog_ptr XSD_;
		Util::WkFontsWidget *WkFontsWidget_;

		Util::ShortcutManager *ShortcutsMgr_;

		std::shared_ptr<Storage> Storage_;
		std::shared_ptr<AccountsManager> AccsMgr_;

		ProgressManager *ProgressMgr_;
		ComposeMessageTabFactory *ComposeTabFactory_;
		MsgTemplatesManager *TemplatesMgr_;

		ICoreProxy_ptr Proxy_;
	public:
		void Init (ICoreProxy_ptr);
		void SecondInit ();
		QByteArray GetUniqueID () const;
		void Release ();
		QString GetName () const;
		QString GetInfo () const;
		QIcon GetIcon () const;

		TabClasses_t GetTabClasses () const;
		void TabOpenRequested (const QByteArray&);

		Util::XmlSettingsDialog_ptr GetSettingsDialog () const;

		QAbstractItemModel* GetRepresentation () const;

		void SetShortcut (const QString& id, const QKeySequences_t& sequences);
		QMap<QString, LC::ActionInfo> GetActionInfo () const;
	private slots:
		void handleNewTab (const QString& name, QWidget*);
	};
}
}
