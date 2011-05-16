/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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

#include "hili.h"
#include <QCoreApplication>
#include <QIcon>
#include <QMessageBox>
#include <QPushButton>
#include <QSettings>
#include <interfaces/imessage.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include "xmlsettingsmanager.h"

namespace LeechCraft
{
namespace Azoth
{
namespace HiLi
{
	void Plugin::Init (ICoreProxy_ptr)
	{
		XmlSettingsDialog_.reset (new Util::XmlSettingsDialog ());
		XmlSettingsDialog_->RegisterObject (&XmlSettingsManager::Instance (),
				"azothhilisettings.xml");
		
		XmlSettingsManager::Instance ().RegisterObject ("HighlightRegexps",
				this, "handleRegexpsChanged");
	}

	void Plugin::SecondInit ()
	{
	}	

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Azoth.HiLi";
	}

	void Plugin::Release ()
	{
	}

	QString Plugin::GetName () const
	{
		return "Azoth HiLi";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Azoth Hili allows to customize the settings of highlights in conferences.");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon ();
	}

	QSet<QByteArray> Plugin::GetPluginClasses () const
	{
		QSet<QByteArray> result;
		result << "org.LeechCraft.Plugins.Azoth.Plugins.IGeneralPlugin";
		return result;
	}
	
	Util::XmlSettingsDialog_ptr Plugin::GetSettingsDialog () const
	{
		return XmlSettingsDialog_;
	}
	
	void Plugin::hookIsHighlightMessage (IHookProxy_ptr proxy, QObject *msgObj)
	{
		if (RegexpsCache_.isEmpty ())
			return;

		IMessage *msg = qobject_cast<IMessage*> (msgObj);
		if (msg->GetMessageType () != IMessage::MTMUCMessage)
			return;
		
		bool isHighlight = false;
		const QString& body = msg->GetBody ();
		Q_FOREACH (const QRegExp& rx, RegexpsCache_)
			if (body.contains (rx))
			{
				isHighlight = true;
				break;
			}

		if (isHighlight)
		{
			proxy->CancelDefault ();
			proxy->SetReturnValue (true);
		}
	}
	
	void Plugin::handleRegexpsChanged ()
	{
		RegexpsCache_.clear ();
		const QStringList& strings = XmlSettingsManager::Instance ()
				.property ("HighlightRegexps").toString ()
					.split ('\n', QString::SkipEmptyParts);
		Q_FOREACH (QString string, strings)
		{
			string = string.trimmed ();
			string.prepend (".*");
			string.append (".*");
			RegexpsCache_ << QRegExp (string, Qt::CaseInsensitive, QRegExp::RegExp2);
		}
	}
}
}
}

Q_EXPORT_PLUGIN2 (leechcraft_azoth_hili, LeechCraft::Azoth::HiLi::Plugin);
