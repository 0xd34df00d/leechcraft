/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "fxb.h"
#include <QIcon>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include <util/util.h>
#include "document.h"
#include "xmlsettingsmanager.h"

namespace LC
{
namespace Monocle
{
namespace FXB
{
	void Plugin::Init (ICoreProxy_ptr)
	{
		Util::InstallTranslator ("monocle_fxb");

		XSD_.reset (new Util::XmlSettingsDialog);
		XSD_->RegisterObject (&XmlSettingsManager::Instance (), "monoclefxbsettings.xml");
	}

	void Plugin::SecondInit ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Monocle.FXB";
	}

	void Plugin::Release ()
	{
	}

	QString Plugin::GetName () const
	{
		return "Monocle FXB";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("FictionBook (fb2) backend for Monocle.");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon ();
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

	auto Plugin::CanLoadDocument (const QString& file) -> LoadCheckResult
	{
		return file.toLower ().endsWith (".fb2") ?
				LoadCheckResult::Can :
				LoadCheckResult::Cannot;
	}

	IDocument_ptr Plugin::LoadDocument (const QString& file)
	{
		return std::make_shared<Document> (file, this);
	}

	QStringList Plugin::GetSupportedMimes () const
	{
		return { "application/x-fictionbook+xml", "application/x-fictionbook" };
	}

	QList<IKnowFileExtensions::ExtInfo> Plugin::GetKnownFileExtensions () const
	{
		return
		{
			{
				tr ("FB2 books"),
				{ "fb2" }
			}
		};
	}
}
}
}

LC_EXPORT_PLUGIN (leechcraft_monocle_fxb, LC::Monocle::FXB::Plugin);
