/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "keywords.h"
#include <QApplication>
#include <QKeyEvent>
#include <QDebug>
#include <util/util.h>
#include <xmlsettingsdialog/basesettingsmanager.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/iiconthememanager.h>
#include "keywordsmanagerwidget.h"

namespace LC
{
namespace Poshuku
{
namespace Keywords
{
	using XmlSettingsManager = Util::SingletonSettingsManager<"Poshuku_Keywords">;

	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Util::InstallTranslator ("poshuku_keywords");

		CoreProxy_ = proxy;

		auto model = new QStandardItemModel;
		model->setHorizontalHeaderLabels ({ tr ("Keyword"), tr ("Url") });

		QSettings keywords { QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_Poshuku_Keywords" };

		for (const auto& keyword : keywords.allKeys ())
		{
			const auto& url = keywords.value (keyword).toString ();
			const QList<QStandardItem*> items
			{
				new QStandardItem { keyword },
				new QStandardItem { url }
			};

			model->appendRow (items);
			UpdateKeywords (keyword, url);
		}

		SettingsDialog_ = std::make_shared<Util::XmlSettingsDialog> ();
		SettingsDialog_->RegisterObject (&XmlSettingsManager::Instance (),
				"poshukukeywordssettings.xml");
		SettingsDialog_->SetCustomWidget ("KeywordsManagerWidget",
				new KeywordsManagerWidget { model, this });
	}

	void Plugin::SecondInit ()
	{
	}

	void Plugin::Release ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Poshuku.Keywords";
	}

	QString Plugin::GetName () const
	{
		return "Poshuku Keywords";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("URL keywords support for the Poshuku browser.");
	}

	QIcon Plugin::GetIcon () const
	{
		return GetProxyHolder ()->GetIconThemeManager ()->GetPluginIcon ();
	}

	QSet<QByteArray> Plugin::GetPluginClasses () const
	{
		QSet<QByteArray> result;
		result << "org.LeechCraft.Poshuku.Plugins/1.0";
		return result;
	}

	Util::XmlSettingsDialog_ptr Plugin::GetSettingsDialog () const
	{
		return SettingsDialog_;
	}

	void Plugin::UpdateKeywords (const QString& keyword, const QString& url)
	{
		Keywords2Urls_ [keyword] = url;
	}

	void Plugin::RemoveKeyword (const QString& keyword)
	{
		Keywords2Urls_.remove (keyword);
	}

	void Plugin::hookURLEditReturnPressed (LC::IHookProxy_ptr proxy, QObject*)
	{
		const auto& text = proxy->GetValue ("Text").toString ();
		if (text.isEmpty () || !text.contains (' '))
			return;

		const auto& redirect = Keywords2Urls_.value (text.section (' ', 0, 0));
		if (redirect.isEmpty ())
			return;

		const auto& query = text.section (' ', 1).toUtf8 ();
		const auto& encoded = query.toPercentEncoding ();
		proxy->SetValue ("Text", redirect.arg (QString::fromUtf8 (encoded)));
	}
}
}
}

LC_EXPORT_PLUGIN (leechcraft_poshuku_keywords, LC::Poshuku::Keywords::Plugin);
