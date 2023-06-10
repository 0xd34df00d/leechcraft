/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <interfaces/iinfo.h>
#include <interfaces/ihavetabs.h>
#include <interfaces/ihavesettings.h>
#include <interfaces/ipluginready.h>
#include <interfaces/iactionsexporter.h>
#include <interfaces/core/ihookproxy.h>
#include <interfaces/ihaverecoverabletabs.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h>

namespace LC
{
namespace Blogique
{
	class Plugin : public QObject
				, public IInfo
				, public IHaveTabs
				, public IHaveSettings
				, public IPluginReady
				, public IActionsExporter
				, public IHaveRecoverableTabs
	{
		Q_OBJECT
		Q_INTERFACES (IInfo IHaveTabs IHaveSettings IPluginReady IActionsExporter
				IHaveRecoverableTabs)

		LC_PLUGIN_METADATA ("org.LeechCraft.Blogique")

		TabClasses_t TabClasses_;
		Util::XmlSettingsDialog_ptr XmlSettingsDialog_;

		QAction *ExportAction_ = nullptr;
	public:
		void Init (ICoreProxy_ptr proxy);
		void SecondInit ();
		QByteArray GetUniqueID () const;
		void Release ();
		QString GetName () const;
		QString GetInfo () const;
		QIcon GetIcon () const;

		TabClasses_t GetTabClasses () const;
		void TabOpenRequested (const QByteArray& tabClass);

		Util::XmlSettingsDialog_ptr GetSettingsDialog () const;

		QSet<QByteArray> GetExpectedPluginClasses () const;
		void AddPlugin (QObject* plugin);

		QList<QAction*> GetActions (ActionsEmbedPlace area) const;

		void RecoverTabs (const QList<TabRecoverInfo>& infos);
		bool HasSimilarTab (const QByteArray&, const QList<QByteArray>&) const;
	private:
		void CreateTab ();
	signals:
		void gotActions (QList<QAction*> actions, LC::ActionsEmbedPlace area);
	};
}
}
