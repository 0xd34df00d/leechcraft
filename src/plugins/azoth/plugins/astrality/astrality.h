/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <interfaces/iinfo.h>
#include <interfaces/iplugin2.h>
#include <interfaces/azoth/iprotocolplugin.h>

namespace Tp
{
	class PendingOperation;
}

namespace LC
{
namespace Azoth
{
namespace Astrality
{
	class CMWrapper;

	class Plugin : public QObject
					, public IInfo
					, public IPlugin2
					, public IProtocolPlugin
	{
		Q_OBJECT
		Q_INTERFACES (IInfo IPlugin2 LC::Azoth::IProtocolPlugin);

		ICoreProxy_ptr Proxy_;
		QList<CMWrapper*> Wrappers_;
	public:
		void Init (ICoreProxy_ptr);
		void SecondInit ();
		void Release ();
		QByteArray GetUniqueID () const;
		QString GetName () const;
		QString GetInfo () const;
		QIcon GetIcon () const;

		QSet<QByteArray> GetPluginClasses () const;

		QObject* GetQObject ();
		QList<QObject*> GetProtocols () const;
	public slots:
		void initPlugin (QObject*);
	private slots:
		void handleListNames (Tp::PendingOperation*);
		void handleProtoWrappers (const QList<QObject*>&);
	signals:
		void gotEntity (const LC::Entity&);

		void gotNewProtocols (const QList<QObject*>&);
	};
}
}
}
