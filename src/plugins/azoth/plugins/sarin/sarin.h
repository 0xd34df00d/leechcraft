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

namespace LC
{
namespace Azoth
{
namespace Sarin
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
		void Init (ICoreProxy_ptr);
		void SecondInit ();
		QByteArray GetUniqueID () const;
		void Release ();
		QString GetName () const;
		QString GetInfo () const;
		QIcon GetIcon () const;

		QString GetDiagInfoString () const;

		QSet<QByteArray> GetPluginClasses () const;

		QObject* GetQObject ();
		QList<QObject*> GetProtocols () const;
	signals:
		void gotNewProtocols (const QList<QObject*>&);
	};
}
}
}

