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

namespace LC::Monocle
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
		void Init (ICoreProxy_ptr) override;
		void SecondInit () override;
		QByteArray GetUniqueID () const override;
		void Release () override;
		QString GetName () const override;
		QString GetInfo () const override;
		QIcon GetIcon () const override;

		EntityTestHandleResult CouldHandle (const Entity&) const override;
		void Handle (Entity) override;

		Util::XmlSettingsDialog_ptr GetSettingsDialog () const override;

		TabClasses_t GetTabClasses () const override;
		void TabOpenRequested (const QByteArray&) override;

		QSet<QByteArray> GetExpectedPluginClasses () const override;
		void AddPlugin (QObject*) override;

		void RecoverTabs (const QList<TabRecoverInfo>& infos) override;
		bool HasSimilarTab (const QByteArray&, const QList<QByteArray>&) const override;

		QMap<QByteArray, ActionInfo> GetActionInfo () const override;
		void SetShortcut (const QByteArray&, const QKeySequences_t&) override;
	};
}
