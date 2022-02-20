/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <interfaces/iinfo.h>
#include <interfaces/ihavetabs.h>
#include <interfaces/ipluginready.h>
#include <interfaces/ihaverecoverabletabs.h>
#include <interfaces/ientityhandler.h>
#include <interfaces/ihavesettings.h>
#include <interfaces/ihaveshortcuts.h>

namespace LC
{
namespace Monocle
{
	class DocumentTab;

	class Plugin : public QObject
					, public IInfo
					, public IEntityHandler
					, public IHaveSettings
					, public IHaveTabs
					, public IPluginReady
					, public IHaveRecoverableTabs
					, public IHaveShortcuts
	{
		Q_OBJECT
		Q_INTERFACES (IInfo
				IEntityHandler
				IHaveSettings
				IHaveTabs
				IPluginReady
				IHaveRecoverableTabs
				IHaveShortcuts)

		LC_PLUGIN_METADATA ("org.LeechCraft.Monocle")

		Util::XmlSettingsDialog_ptr XSD_;

		TabClassInfo DocTabInfo_;
	public:
		void Init (ICoreProxy_ptr);
		void SecondInit ();
		QByteArray GetUniqueID () const;
		void Release ();
		QString GetName () const;
		QString GetInfo () const;
		QIcon GetIcon () const;

		EntityTestHandleResult CouldHandle (const Entity&) const;
		void Handle (Entity);

		Util::XmlSettingsDialog_ptr GetSettingsDialog() const;

		TabClasses_t GetTabClasses () const;
		void TabOpenRequested (const QByteArray&);

		QSet<QByteArray> GetExpectedPluginClasses () const;
		void AddPlugin (QObject*);

		void RecoverTabs (const QList<TabRecoverInfo>& infos);
		bool HasSimilarTab (const QByteArray&, const QList<QByteArray>&) const;

		QMap<QString, ActionInfo> GetActionInfo () const;
		void SetShortcut (const QString&, const QKeySequences_t&);
	};
}
}

