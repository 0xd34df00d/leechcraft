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
#include <interfaces/ihavesettings.h>
#include <interfaces/ihavediaginfo.h>
#include <interfaces/azoth/iprotocolplugin.h>

namespace LC::Azoth::Sarin
{
	class ToxProtocol;

	class Plugin : public QObject
				 , public IInfo
				 , public IHaveDiagInfo
				 , public IPlugin2
				 , public IProtocolPlugin
	{
		Q_OBJECT
		Q_INTERFACES (IInfo IPlugin2 IHaveDiagInfo LC::Azoth::IProtocolPlugin)

		LC_PLUGIN_METADATA ("org.LeechCraft.Azoth.Sarin")

		std::shared_ptr<ToxProtocol> Proto_;
	public:
		void Init (ICoreProxy_ptr) override;
		void SecondInit () override;
		QByteArray GetUniqueID () const override;
		void Release () override;
		QString GetName () const override;
		QString GetInfo () const override;
		QIcon GetIcon () const override;

		QString GetDiagInfoString () const override;

		QSet<QByteArray> GetPluginClasses () const override;

		QObject* GetQObject () override;
		QList<QObject*> GetProtocols () const override;
	signals:
		void gotNewProtocols (const QList<QObject*>&) override;
	};
}

