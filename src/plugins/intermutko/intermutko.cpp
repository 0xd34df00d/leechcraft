/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "intermutko.h"
#include <QIcon>
#include <QNetworkRequest>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include <util/util.h>
#include "xmlsettingsmanager.h"
#include "acceptlangwidget.h"
#include "localeentry.h"

namespace LC
{
namespace Intermutko
{
	void Plugin::Init (ICoreProxy_ptr)
	{
		qRegisterMetaType<QList<LocaleEntry>> ("QList<LocaleEntry>");
#if QT_VERSION_MAJOR == 5
		qRegisterMetaTypeStreamOperators<QList<LocaleEntry>> ();
#endif

		Util::InstallTranslator ("intermutko");

		XSD_ = std::make_shared<Util::XmlSettingsDialog> ();
		XSD_->RegisterObject (&XmlSettingsManager::Instance (), "intermutkosettings.xml");

		AcceptLangWidget_ = new AcceptLangWidget;
		XSD_->SetCustomWidget ("AcceptLangWidget", AcceptLangWidget_);
	}

	void Plugin::SecondInit ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Intermutko";
	}

	void Plugin::Release ()
	{
	}

	QString Plugin::GetName () const
	{
		return "Intermutko";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Provides fine-grained tuning for the Accept-Language HTTP header.");
	}

	QIcon Plugin::GetIcon () const
	{
		return {};
	}

	QSet<QByteArray> Plugin::GetPluginClasses () const
	{
		QSet<QByteArray> result;
		result << "org.LeechCraft.Core.Plugins/1.0";
		return result;
	}

	Util::XmlSettingsDialog_ptr Plugin::GetSettingsDialog () const
	{
		return XSD_;
	}

	void Plugin::hookNAMCreateRequest (LC::IHookProxy_ptr proxy,
			QNetworkAccessManager*, QNetworkAccessManager::Operation*, QIODevice**)
	{
		auto req = proxy->GetValue ("request").value<QNetworkRequest> ();
		if (!req.url ().scheme ().startsWith ("http"))
			return;

		const auto& localeStr = AcceptLangWidget_->GetLocaleString ();
		req.setRawHeader ("Accept-Language",
				localeStr.isEmpty () ?
						" " :
						localeStr.toUtf8 ());
		proxy->SetValue ("request", QVariant::fromValue (req));
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_intermutko, LC::Intermutko::Plugin);
