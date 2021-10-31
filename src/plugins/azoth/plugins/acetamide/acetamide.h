/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <interfaces/iinfo.h>
#include <interfaces/ihavesettings.h>
#include <interfaces/iplugin2.h>
#include <interfaces/azoth/iprotocolplugin.h>

namespace LC::Azoth::Acetamide
{
	class IrcProtocol;
	class NickServIdentifyManager;

	class Plugin : public QObject
					, public IInfo
					, public IHaveSettings
					, public IPlugin2
					, public IProtocolPlugin
	{
		Q_OBJECT
		Q_INTERFACES (IInfo IHaveSettings IPlugin2
				LC::Azoth::IProtocolPlugin)

		LC_PLUGIN_METADATA ("org.LeechCraft.Azoth.Acetamide")

		Util::XmlSettingsDialog_ptr SettingsDialog_;
		std::shared_ptr<NickServIdentifyManager> IdentifyManager_;

		std::shared_ptr<IrcProtocol> IrcProtocol_;
	public:
		void Init (ICoreProxy_ptr) override;
		void SecondInit () override;
		void Release () override;
		QByteArray GetUniqueID () const override;
		QString GetName () const override;
		QString GetInfo () const override;
		QIcon GetIcon () const override;

		QSet<QByteArray> GetPluginClasses () const override;

		QObject* GetQObject () override;
		QList<QObject*> GetProtocols () const override;
		Util::XmlSettingsDialog_ptr GetSettingsDialog () const override;
	public slots:
		void initPlugin (QObject*);
	signals:
		void gotNewProtocols (const QList<QObject*>&) override;
	};
}
