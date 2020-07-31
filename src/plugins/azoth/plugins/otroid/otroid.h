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
#include <interfaces/core/ihookproxy.h>
#include <interfaces/ihavesettings.h>
#include <interfaces/azoth/iproxyobject.h>
#include <interfaces/azoth/imessage.h>

class QTranslator;

namespace LC
{
namespace Azoth
{
class ICLEntry;

namespace OTRoid
{
	class FPManager;
	class PrivKeyManager;
	class OtrHandler;

	class Plugin : public QObject
				 , public IInfo
				 , public IPlugin2
				 , public IHaveSettings
	{
		Q_OBJECT
		Q_INTERFACES (IInfo IPlugin2 IHaveSettings)

		LC_PLUGIN_METADATA ("org.LeechCraft.Azoth.OTRoid")

		ICoreProxy_ptr CoreProxy_;
		IProxyObject *AzothProxy_;

		Util::XmlSettingsDialog_ptr XSD_;

		OtrHandler *OtrHandler_ = nullptr;
		FPManager *FPManager_ = nullptr;
		PrivKeyManager *PKManager_ = nullptr;
	public:
		void Init (ICoreProxy_ptr);
		void SecondInit ();
		QByteArray GetUniqueID () const;
		void Release ();
		QString GetName () const;
		QString GetInfo () const;
		QIcon GetIcon () const;

		QSet<QByteArray> GetPluginClasses () const;

		Util::XmlSettingsDialog_ptr GetSettingsDialog () const;

		FPManager* GetFPManager () const;
	public slots:
		void initPlugin (QObject*);

		void hookEntryActionAreasRequested (LC::IHookProxy_ptr proxy,
				QObject *action,
				QObject *entry);
		void hookEntryActionsRemoved (LC::IHookProxy_ptr proxy,
				QObject *entry);
		void hookEntryActionsRequested (LC::IHookProxy_ptr proxy,
				QObject *entry);
		void hookGotMessage (LC::IHookProxy_ptr proxy,
				QObject *message);
		void hookMessageCreated (LC::IHookProxy_ptr proxy,
				QObject *chatTab,
				QObject *message);
	};
}
}
}
