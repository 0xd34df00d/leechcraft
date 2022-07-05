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
#include <interfaces/iplugin2.h>
#include <interfaces/iactionsexporter.h>
#include <interfaces/lmp/ilmpplugin.h>

namespace LC::LMP::PPL
{
	class Plugin : public QObject
				 , public IInfo
				 , public IPlugin2
				 , public IActionsExporter
				 , public ILMPPlugin
	{
		Q_OBJECT
		Q_INTERFACES (IInfo
				IPlugin2
				IActionsExporter
				LC::LMP::ILMPPlugin)

		LC_PLUGIN_METADATA ("org.LeechCraft.LMP.PPL")

		ICoreProxy_ptr Proxy_;
		ILMPProxy_ptr LMPProxy_ = {};

		QAction *ActionSync_;
	public:
		void Init (ICoreProxy_ptr) override;
		void SecondInit () override;
		void Release () override;
		QByteArray GetUniqueID () const override;
		QString GetName () const override;
		QString GetInfo () const override;
		QIcon GetIcon () const override;

		QSet<QByteArray> GetPluginClasses () const override;

		QList<QAction*> GetActions (ActionsEmbedPlace area) const override;
		QMap<QString, QList<QAction*>> GetMenuActions () const override;

		void SetLMPProxy (ILMPProxy_ptr) override;
	signals:
		void gotActions (QList<QAction*>, LC::ActionsEmbedPlace) override;
	};
}
