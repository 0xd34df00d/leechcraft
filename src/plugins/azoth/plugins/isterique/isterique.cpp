/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "isterique.h"
#include <QIcon>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include <util/util.h>
#include <interfaces/imessage.h>
#include "xmlsettingsmanager.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Isterique
{
	void Plugin::Init (ICoreProxy_ptr)
	{
		Util::InstallTranslator ("azoth_isterique");

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
		return QIcon (":/plugins/azoth/plugins/isterique/resources/images/isterique.svg");
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

	void Plugin::hookGotMessage (LeechCraft::IHookProxy_ptr proxy,
			QObject *message)
	{
		IMessage *msg = qobject_cast<IMessage*> (message);
		if (!msg)
			return;

		if (msg->GetMessageType () != IMessage::MTChatMessage &&
				msg->GetMessageType () != IMessage::MTMUCMessage)
			return;

		if ((msg->GetMessageType () == IMessage::MTChatMessage &&
				!XmlSettingsManager::Instance ().property ("EnableForChats").toBool ()) ||
			(msg->GetMessageType () == IMessage::MTMUCMessage &&
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

LC_EXPORT_PLUGIN (leechcraft_azoth_isterique, LeechCraft::Azoth::Isterique::Plugin);
