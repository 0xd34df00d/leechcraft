/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "hili.h"
#include <QCoreApplication>
#include <QIcon>
#include <QMessageBox>
#include <QTranslator>
#include <xmlsettingsdialog/basesettingsmanager.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include <util/util.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/iiconthememanager.h>
#include <interfaces/azoth/imessage.h>

namespace LC
{
namespace Azoth
{
namespace HiLi
{
	using XmlSettingsManager = Util::SingletonSettingsManager<"Azoth_HiLi">;

	void Plugin::Init (ICoreProxy_ptr)
	{
		Util::InstallTranslator ("azoth_hili");

		XmlSettingsDialog_ = std::make_shared<Util::XmlSettingsDialog> ();
		XmlSettingsDialog_->RegisterObject (&XmlSettingsManager::Instance (),
				"azothhilisettings.xml");

		XmlSettingsManager::Instance ().RegisterObject ("HighlightRegexps",
				this, "handleRegexpsChanged");
		handleRegexpsChanged ();
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
		return tr ("Azoth Hili allows one to customize the settings of highlights in conferences.");
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
		return XmlSettingsDialog_;
	}

	void Plugin::hookIsHighlightMessage (IHookProxy_ptr proxy, QObject *msgObj)
	{
		if (RegexpsCache_.isEmpty ())
			return;

		IMessage *msg = qobject_cast<IMessage*> (msgObj);
		if (msg->GetMessageType () != IMessage::Type::MUCMessage)
			return;

		const auto& body = msg->GetBody ();
		if (body.size () > 1024)
		{
			qWarning () << Q_FUNC_INFO
					<< "too big string to handle:"
					<< body.size ();
			return;
		}

		if (std::ranges::any_of (RegexpsCache_, [&body] (const QRegularExpression& rx) { return body.contains (rx); }))
		{
			proxy->CancelDefault ();
			proxy->SetReturnValue (true);
		}
	}

	void Plugin::handleRegexpsChanged ()
	{
		RegexpsCache_.clear ();
		auto strings = XmlSettingsManager::Instance ().property ("HighlightRegexps").toStringList ();
		for (auto&& string : strings)
		{
			string = std::move (string).trimmed ();
			if (!string.isEmpty ())
				RegexpsCache_ << QRegularExpression { string, QRegularExpression::CaseInsensitiveOption };
		}
	}
}
}
}

LC_EXPORT_PLUGIN (leechcraft_azoth_hili, LC::Azoth::HiLi::Plugin);
