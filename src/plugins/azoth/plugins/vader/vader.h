/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <interfaces/iinfo.h>
#include <interfaces/ihavesettings.h>
#include <interfaces/iplugin2.h>
#include <interfaces/core/ihookproxy.h>
#include <interfaces/azoth/iprotocolplugin.h>

class QAction;

namespace LC
{
namespace Azoth
{
class IProxyObject;

namespace Vader
{
	class MRIMProtocol;

	class Plugin : public QObject
					, public IInfo
					, public IHaveSettings
					, public IPlugin2
					, public IProtocolPlugin
	{
		Q_OBJECT
		Q_INTERFACES (IInfo IHaveSettings IPlugin2 LC::Azoth::IProtocolPlugin)

		LC_PLUGIN_METADATA ("org.LeechCraft.Azoth.Vader")

		ICoreProxy_ptr CoreProxy_;
		Util::XmlSettingsDialog_ptr XSD_;

		QMap<QObject*, QList<QAction*>> EntryServices_;

		std::shared_ptr<MRIMProtocol> Proto_;

		IProxyObject *AzothProxy_ = nullptr;
	public:
		void Init (ICoreProxy_ptr);
		void SecondInit ();
		void Release ();
		QByteArray GetUniqueID () const;
		QString GetName () const;
		QString GetInfo () const;
		QIcon GetIcon () const;

		Util::XmlSettingsDialog_ptr GetSettingsDialog () const;

		QSet<QByteArray> GetPluginClasses () const;

		QObject* GetQObject ();
		QList<QObject*> GetProtocols () const;
	public slots:
		void initPlugin (QObject*);

		void hookEntryActionAreasRequested (LC::IHookProxy_ptr proxy,
				QObject *action,
				QObject *entry);
		void hookEntryActionsRequested (LC::IHookProxy_ptr proxy,
				QObject *entry);
		void entryServiceRequested ();
	signals:
		void gotNewProtocols (const QList<QObject*>&);
	};
}
}
}
