/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "fua.h"
#include <QStandardItemModel>
#include <QUrl>
#include <QSettings>
#include <QFile>
#include <QCoreApplication>
#include <util/util.h>
#include <util/sll/parsejson.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/iiconthememanager.h>
#include <interfaces/poshuku/iproxyobject.h>
#include "settings.h"
#include "xmlsettingsmanager.h"

namespace LC
{
namespace Util
{
	class XmlSettingsDialog;
};
namespace Poshuku
{
namespace Fua
{
	void FUA::Init (ICoreProxy_ptr)
	{
		Util::InstallTranslator ("poshuku_fua");

		Model_ = std::make_shared<QStandardItemModel> ();
		Model_->setHorizontalHeaderLabels ({ tr ("Domain"), tr ("Agent"), tr ("Identification string") });

		XmlSettingsDialog_ = std::make_shared<Util::XmlSettingsDialog> ();
		XmlSettingsDialog_->RegisterObject (XmlSettingsManager::Instance (),
				"poshukufuasettings.xml");
		XmlSettingsDialog_->SetCustomWidget ("Settings", new Settings { Model_.get (), this });
	}

	void FUA::SecondInit ()
	{
	}

	void FUA::Release ()
	{
	}

	QByteArray FUA::GetUniqueID () const
	{
		return "org.LeechCraft.Poshuku.FUA";
	}

	QString FUA::GetName () const
	{
		return "Poshuku FUA";
	}

	QString FUA::GetInfo () const
	{
		return tr ("Allows one to set fake user agents for different sites.");
	}

	QIcon FUA::GetIcon () const
	{
		return GetProxyHolder ()->GetIconThemeManager ()->GetPluginIcon ();
	}

	QSet<QByteArray> FUA::GetPluginClasses () const
	{
		QSet<QByteArray> result;
		result << "org.LeechCraft.Poshuku.Plugins/1.0";
		return result;
	}

	Util::XmlSettingsDialog_ptr FUA::GetSettingsDialog () const
	{
		return XmlSettingsDialog_;
	}

	void FUA::hookUserAgentForUrlRequested (LC::IHookProxy_ptr proxy,
			const QUrl& url)
	{
		const auto& host = url.host ();
		for (int i = 0; i < Model_->rowCount (); ++i)
		{
			const auto item = Model_->item (i);
			QRegExp re { item->text (), Qt::CaseSensitive, QRegExp::Wildcard };
			if (re.exactMatch (host))
			{
				proxy->CancelDefault ();
				proxy->SetReturnValue (Model_->item (i, 2)->text ());
				return;
			}
		}
	}

	void FUA::Save () const
	{
		QSettings settings { QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_Poshuku_FUA" };
		settings.beginWriteArray ("Fakes");
		settings.remove ("");
		for (int i = 0; i < Model_->rowCount (); ++i)
		{
			settings.setArrayIndex (i);
			settings.setValue ("domain", Model_->item (i, 0)->text ());
			settings.setValue ("identification", Model_->item (i, 2)->text ());
		}
		settings.endArray ();
	}

	const QList<QPair<QString, QString>>& FUA::GetBrowser2ID () const
	{
		return Browser2ID_;
	}

	const QMap<QString, QString>& FUA::GetBackLookupMap () const
	{
		return BackLookup_;
	}

	namespace
	{
		QList<QPair<QString, QString>> LoadFromJson ()
		{
			QFile file { ":/poshuku/fua/resources/data/substs.json" };
			if (!file.open (QIODevice::ReadOnly))
			{
				qWarning () << Q_FUNC_INFO
						<< "cannot open file:"
						<< file.errorString ();
				return {};
			}

			const auto& json = Util::ParseJson (&file, Q_FUNC_INFO);

			QList<QPair<QString, QString>> result;

			for (const auto& itemVar : json.toList ())
			{
				const auto& map = itemVar.toMap ();
				const auto& name = map ["n"].toString ();
				const auto& value = map ["v"].toString ();
				if (name.isEmpty () || value.isEmpty ())
				{
					qWarning () << Q_FUNC_INFO
							<< "cannot parse"
							<< itemVar;
					continue;
				}

				result.append ({ name, value });
			}

			return result;
		}

		QMap<QString, QString> MakeLookupMap (const QList<QPair<QString, QString>>& pairs)
		{
			QMap<QString, QString> result;
			for (const auto& pair : pairs)
				result [pair.second] = pair.first;
			return result;
		}
	}

	void FUA::initPlugin (QObject*)
	{
		Browser2ID_ = LoadFromJson ();
		Browser2ID_.prepend ({ tr ("LeechCraft (this machine)"), "" });

		BackLookup_ = MakeLookupMap (Browser2ID_);

		QSettings settings { QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_Poshuku_FUA" };
		int size = settings.beginReadArray ("Fakes");

		for (int i = 0; i < size; ++i)
		{
			settings.setArrayIndex (i);
			const auto& domain = settings.value ("domain").toString ();
			const auto& identification = settings.value ("identification").toString ();
			const QList<QStandardItem*> items
			{
				new QStandardItem { domain },
				new QStandardItem { BackLookup_ [identification] },
				new QStandardItem { identification }
			};
			Model_->appendRow (items);
		}
		settings.endArray ();
	}
}
}
}

LC_EXPORT_PLUGIN (leechcraft_poshuku_fua, LC::Poshuku::Fua::FUA);
