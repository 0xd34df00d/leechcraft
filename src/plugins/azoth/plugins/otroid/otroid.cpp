/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "otroid.h"
#include <QCoreApplication>
#include <QIcon>
#include <QAction>
#include <QTranslator>

extern "C"
{
#include <libotr/proto.h>
}

#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/iiconthememanager.h>
#include <interfaces/azoth/iprotocol.h>
#include <interfaces/azoth/iaccount.h>
#include <interfaces/azoth/iclentry.h>
#include <interfaces/azoth/imessage.h>
#include <util/util.h>
#include <util/sys/paths.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include "xmlsettingsmanager.h"
#include "otrhandler.h"
#include "fpmanager.h"
#include "privkeymanager.h"

namespace LC
{
namespace Azoth
{
namespace OTRoid
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Util::InstallTranslator ("azoth_otroid");

		XSD_ = std::make_shared<Util::XmlSettingsDialog> ();
		XSD_->RegisterObject (&XmlSettingsManager::Instance (), "azothotroidsettings.xml");

		CoreProxy_ = proxy;

		OTRL_INIT;
	}

	void Plugin::SecondInit ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Azoth.OTRoid";
	}

	void Plugin::Release ()
	{
	}

	QString Plugin::GetName () const
	{
		return "Azoth OTRoid";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Azoth OTRoid adds support for Off-the-Record deniable encryption system.");
	}

	QIcon Plugin::GetIcon () const
	{
		return GetProxyHolder ()->GetIconThemeManager ()->GetPluginIcon ();
	}

	QSet<QByteArray> Plugin::GetPluginClasses () const
	{
		QSet<QByteArray> result;
		result << "org.LeechCraft.Plugins.Azoth.Plugins.IGeneralPlugin";
		return result;
	}

	Util::XmlSettingsDialog_ptr Plugin::GetSettingsDialog () const
	{
		return XSD_;
	}

	FPManager* Plugin::GetFPManager () const
	{
		return FPManager_;
	}

	void Plugin::initPlugin (QObject *obj)
	{
		AzothProxy_ = qobject_cast<IProxyObject*> (obj);

		OtrHandler_ = new OtrHandler (CoreProxy_, AzothProxy_);

		FPManager_ = new FPManager (OtrHandler_->GetUserState (), AzothProxy_);
		connect (FPManager_,
				SIGNAL (fingerprintsChanged ()),
				OtrHandler_,
				SLOT (writeFingerprints ()));
		XSD_->SetDataSource ("KnownFPs", FPManager_->GetModel ());

		PKManager_ = new PrivKeyManager (OtrHandler_->GetUserState (), AzothProxy_);
		connect (PKManager_,
				SIGNAL (keysChanged ()),
				OtrHandler_,
				SLOT (writeKeys ()));
		connect (PKManager_,
				SIGNAL (keysGenerationRequested (QString, QString)),
				OtrHandler_,
				SLOT (generateKeys (QString, QString)));
		connect (OtrHandler_,
				SIGNAL (privKeysChanged ()),
				PKManager_,
				SLOT (reloadAll ()));
		XSD_->SetDataSource ("PrivKeys", PKManager_->GetModel ());
	}

	void Plugin::hookEntryActionAreasRequested (IHookProxy_ptr proxy,
			QObject *action, QObject*)
	{
		OtrHandler_->HandleEntryActionAreasRequested (proxy, action);
	}

	void Plugin::hookEntryActionsRemoved (IHookProxy_ptr,
			QObject *entry)
	{
		OtrHandler_->HandleEntryActionsRemoved (entry);
	}

	void Plugin::hookEntryActionsRequested (IHookProxy_ptr proxy, QObject *entry)
	{
		OtrHandler_->HandleEntryActionsRequested (proxy, entry);
	}

	void Plugin::hookGotMessage (IHookProxy_ptr proxy, QObject *msgObj)
	{
		OtrHandler_->HandleGotMessage (proxy, msgObj);
	}

	void Plugin::hookMessageCreated (IHookProxy_ptr proxy, QObject*, QObject *msgObj)
	{
		IMessage *msg = qobject_cast<IMessage*> (msgObj);
		if (!msg)
		{
			qWarning () << Q_FUNC_INFO
					<< msgObj
					<< "doesn't implement IMessage";
			return;
		}

		OtrHandler_->HandleMessageCreated (proxy, msg);
	}
}
}
}

LC_EXPORT_PLUGIN (leechcraft_azoth_otroid, LC::Azoth::OTRoid::Plugin);
