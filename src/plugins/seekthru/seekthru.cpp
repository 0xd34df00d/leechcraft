/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "seekthru.h"
#include <interfaces/entitytesthandleresult.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/iiconthememanager.h>
#include <interfaces/core/ientitymanager.h>
#include <util/util.h>
#include <util/xpc/util.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include "core.h"
#include "xmlsettingsmanager.h"
#include "searcherslist.h"
#include "wizardgenerator.h"

namespace LC::SeekThru
{
	void SeekThru::Init (ICoreProxy_ptr proxy)
	{
		Proxy_ = proxy;

		Util::InstallTranslator ("seekthru");

		Core::Instance ().SetProxy (proxy);

		connect (&Core::Instance (),
				SIGNAL (error (const QString&)),
				this,
				SLOT (handleError (const QString&)),
				Qt::QueuedConnection);
		connect (&Core::Instance (),
				SIGNAL (categoriesChanged (const QStringList&, const QStringList&)),
				this,
				SIGNAL (categoriesChanged (const QStringList&, const QStringList&)));

		Core::Instance ().DoDelayedInit ();

		XmlSettingsDialog_.reset (new Util::XmlSettingsDialog ());
		XmlSettingsDialog_->RegisterObject (&XmlSettingsManager::Instance (),
				"seekthrusettings.xml");

		auto searchersList = new SearchersList { proxy };
		XmlSettingsDialog_->SetCustomWidget ("SearchersList", searchersList);
	}

	void SeekThru::SecondInit ()
	{
	}

	void SeekThru::Release ()
	{
		XmlSettingsDialog_.reset ();
	}

	QByteArray SeekThru::GetUniqueID () const
	{
		return "org.LeechCraft.SeekThru";
	}

	QString SeekThru::GetName () const
	{
		return "SeekThru";
	}

	QString SeekThru::GetInfo () const
	{
		return tr ("Search via OpenSearch-aware search providers.");
	}

	QIcon SeekThru::GetIcon () const
	{
		return Proxy_->GetIconThemeManager ()->GetPluginIcon ();
	}

	QStringList SeekThru::Provides () const
	{
		return QStringList ("search");
	}

	QStringList SeekThru::Needs () const
	{
		return QStringList ("http");
	}

	QStringList SeekThru::Uses () const
	{
		return QStringList ("webbrowser");
	}

	void SeekThru::SetProvider (QObject *object, const QString& feature)
	{
		Core::Instance ().SetProvider (object, feature);
	}

	QStringList SeekThru::GetCategories () const
	{
		return Core::Instance ().GetCategories ();
	}

	QList<IFindProxy_ptr> SeekThru::GetProxy (const LC::Request& r)
	{
		QList<IFindProxy_ptr> result;
		result << Core::Instance ().GetProxy (r);
		return result;
	}

	std::shared_ptr<LC::Util::XmlSettingsDialog> SeekThru::GetSettingsDialog () const
	{
		return XmlSettingsDialog_;
	}

	EntityTestHandleResult SeekThru::CouldHandle (const Entity& e) const
	{
		return Core::Instance ().CouldHandle (e) ?
				EntityTestHandleResult (EntityTestHandleResult::PIdeal) :
				EntityTestHandleResult ();
	}

	void SeekThru::Handle (Entity e)
	{
		Core::Instance ().Handle (e);
	}

	QString SeekThru::GetFilterVerb () const
	{
		return tr ("Search in OpenSearch engines");
	}

	QList<SeekThru::FilterVariant> SeekThru::GetFilterVariants (const QVariant&) const
	{
		QList<FilterVariant> result;
		for (const auto& cat : Core::Instance ().GetCategories ())
			result << FilterVariant
				{
					cat.toUtf8 (),
					cat,
					tr ("Search this term in OpenSearch engines in category %1.").arg (cat),
					{}
				};
		return result;
	}

	QList<QWizardPage*> SeekThru::GetWizardPages () const
	{
		return WizardGenerator {}.GetPages ();
	}

	ISyncProxy* SeekThru::GetSyncProxy ()
	{
		// TODO ISyncProxy
		return nullptr;
	}

	void SeekThru::handleError (const QString& error)
	{
		Proxy_->GetEntityManager ()->HandleEntity (Util::MakeNotification ("SeekThru", error, Priority::Critical));
	}
}

LC_EXPORT_PLUGIN (leechcraft_seekthru, LC::SeekThru::SeekThru);
