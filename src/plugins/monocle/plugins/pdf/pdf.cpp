/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "pdf.h"
#include <QIcon>
#include <poppler-version.h>
#include <util/util.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/iiconthememanager.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include "xmlsettingsmanager.h"
#include "document.h"

namespace LC
{
namespace Monocle
{
namespace PDF
{
	void Plugin::Init (ICoreProxy_ptr)
	{
		Util::InstallTranslator ("monocle_pdf");

		XSD_ = std::make_shared<Util::XmlSettingsDialog> ();
		XSD_->RegisterObject (&XmlSettingsManager::Instance (), "monoclepdfsettings.xml");
	}

	void Plugin::SecondInit ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Monocle.PDF";
	}

	void Plugin::Release ()
	{
	}

	QString Plugin::GetName () const
	{
		return "Monocle PDF";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("PDF backend for Monocle.");
	}

	QIcon Plugin::GetIcon () const
	{
		return GetProxyHolder ()->GetIconThemeManager ()->GetPluginIcon ();
	}

	QSet<QByteArray> Plugin::GetPluginClasses () const
	{
		QSet<QByteArray> result;
		result << "org.LeechCraft.Monocle.IBackendPlugin";
		return result;
	}

	Util::XmlSettingsDialog_ptr Plugin::GetSettingsDialog () const
	{
		return XSD_;
	}

	QString Plugin::GetDiagInfoString () const
	{
		return QString { "Built with poppler " POPPLER_VERSION };
	}

	auto Plugin::CanLoadDocument (const QString& file) -> LoadCheckResult
	{
		return Document (file, this).IsValid () ?
				LoadCheckResult::Can :
				LoadCheckResult::Cannot;
	}

	IDocument_ptr Plugin::LoadDocument (const QString& file)
	{
		return std::make_shared<Document> (file, this);
	}

	QStringList Plugin::GetSupportedMimes () const
	{
		return { "application/pdf" };
	}

	bool Plugin::IsThreaded () const
	{
		return true;
	}

	QList<IKnowFileExtensions::ExtInfo> Plugin::GetKnownFileExtensions () const
	{
		return
		{
			{
				tr ("PDF files"),
				{ "pdf" }
			}
		};
	}
}
}
}

LC_EXPORT_PLUGIN (leechcraft_monocle_pdf, LC::Monocle::PDF::Plugin);
