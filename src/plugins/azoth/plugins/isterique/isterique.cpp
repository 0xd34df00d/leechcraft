/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "isterique.h"
#include <QIcon>
#include <xmlsettingsdialog/basesettingsmanager.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include <interfaces/core/iiconthememanager.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/azoth/imessage.h>

namespace LC
{
namespace Azoth
{
namespace Isterique
{
	using XmlSettingsManager = Util::SingletonSettingsManager<"Azoth_Isterique">;

	void Plugin::Init (ICoreProxy_ptr)
	{
		SettingsDialog_.reset (new Util::XmlSettingsDialog);
		SettingsDialog_->RegisterObject (&XmlSettingsManager::Instance (),
				"azothisteriquesettings.xml");
	}

	void Plugin::SecondInit ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Azoth.Isterique";
	}

	void Plugin::Release ()
	{
	}

	QString Plugin::GetName () const
	{
		return "Azoth Isterique";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Azoth Isterique fixes excessive CAPS LOCK usage in incoming messages.");
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
		return SettingsDialog_;
	}

	void Plugin::hookGotMessage (LC::IHookProxy_ptr, QObject *message)
	{
		IMessage *msg = qobject_cast<IMessage*> (message);
		if (!msg)
			return;

		if (msg->GetMessageType () != IMessage::Type::ChatMessage &&
				msg->GetMessageType () != IMessage::Type::MUCMessage)
			return;

		if ((msg->GetMessageType () == IMessage::Type::ChatMessage &&
				!XmlSettingsManager::Instance ().property ("EnableForChats").toBool ()) ||
			(msg->GetMessageType () == IMessage::Type::MUCMessage &&
				!XmlSettingsManager::Instance ().property ("EnableForMUCs").toBool ()))
			return;

		QString str = msg->GetBody ();
		if (str.length () < 3)
			return;

		int caps = 0;
		int alphaLength = 0;
		for (int i = 0, size = str.length (); i < size; ++i)
		{
			if (!str.at (i).isLetter ())
				continue;

			++alphaLength;
			if (str.at (i).isUpper ())
				++caps;
		}

		double ratio = static_cast<double> (XmlSettingsManager::Instance ()
					.property ("CapsPercentage").toInt ()) / 100;
		if (alphaLength > 3 &&
				static_cast<double> (caps) / alphaLength < ratio)
			return;

		for (int i = 0, size = str.length (); i < size; ++i)
			str [i] = str.at (i).toLower ();

		msg->SetBody (str);
	}
}
}
}

LC_EXPORT_PLUGIN (leechcraft_azoth_isterique, LC::Azoth::Isterique::Plugin);
