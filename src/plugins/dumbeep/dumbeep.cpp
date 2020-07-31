/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "dumbeep.h"
#include <interfaces/entitytesthandleresult.h>
#include <QIcon>
#include <QUrl>
#include <QFileInfo>
#include <QProcess>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include "xmlsettingsmanager.h"

namespace LC
{
namespace Dumbeep
{
	void Plugin::Init (ICoreProxy_ptr)
	{
		XSD_ = std::make_shared<Util::XmlSettingsDialog> ();
		XSD_->RegisterObject (&XmlSettingsManager::Instance (), "dumbeepsettings.xml");
	}

	void Plugin::SecondInit ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Dumbeep";
	}

	void Plugin::Release ()
	{
	}

	QString Plugin::GetName () const
	{
		return "Dumbeep";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("A simple audio notification backend for those fearing LMP.");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon ();
	}

	Util::XmlSettingsDialog_ptr Plugin::GetSettingsDialog () const
	{
		return XSD_;
	}

	namespace
	{
		QString GetPath (const Entity& e)
		{
			QString path = e.Entity_.toString ();
			const QUrl& url = e.Entity_.toUrl ();
			if (path.isEmpty () &&
						url.isValid () &&
						url.scheme () == "file")
				path = url.toLocalFile ();
			return path;
		}
	}

	EntityTestHandleResult Plugin::CouldHandle (const Entity& e) const
	{
		if (XmlSettingsManager::Instance ().property ("PlayerCommand").toString ().isEmpty ())
			return EntityTestHandleResult ();

		if (!(e.Parameters_ & Internal))
			return EntityTestHandleResult ();

		const auto& path = GetPath (e);

		if (!path.isEmpty ())
		{
			const QStringList goodExt = { "mp3", "ogg", "wav", "flac" };
			const QFileInfo fi = QFileInfo (path);
			if (fi.exists () && goodExt.contains (fi.suffix ()))
				return EntityTestHandleResult (EntityTestHandleResult::PNormal);
			else
				return EntityTestHandleResult ();
		}

		return EntityTestHandleResult ();
	}

	void Plugin::Handle (Entity e)
	{
		const auto& path = GetPath (e);

		const auto& commandStr = XmlSettingsManager::Instance ()
				.property ("PlayerCommand").toString ();
		const auto& parts = commandStr.split (' ', Qt::SkipEmptyParts);
		if (parts.isEmpty ())
			return;

		QProcess::startDetached (parts.at (0), parts.mid (1) << path);
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_dumbeep, LC::Dumbeep::Plugin);
