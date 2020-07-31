/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QObject>
#include <QMap>
#include <interfaces/iinfo.h>
#include <interfaces/ihavesettings.h>
#include <interfaces/iplugin2.h>
#include <interfaces/core/ihookproxy.h>

class QStandardItemModel;

namespace LC
{
namespace Poshuku
{
namespace Fua
{
	class Settings;

	class FUA : public QObject
			  , public IInfo
			  , public IPlugin2
			  , public IHaveSettings
	{
		Q_OBJECT
		Q_INTERFACES (IInfo IPlugin2 IHaveSettings)

		LC_PLUGIN_METADATA ("org.LeechCraft.Poshuku.FUA")

		std::shared_ptr<QStandardItemModel> Model_;
		Util::XmlSettingsDialog_ptr XmlSettingsDialog_;

		QList<QPair<QString, QString>> Browser2ID_;
		QMap<QString, QString> BackLookup_;
	public:
		void Init (ICoreProxy_ptr);
		void SecondInit ();
		void Release ();
		QByteArray GetUniqueID () const;
		QString GetName () const;
		QString GetInfo () const;
		QIcon GetIcon () const;

		QSet<QByteArray> GetPluginClasses () const;

		Util::XmlSettingsDialog_ptr GetSettingsDialog () const;

		void Save () const;
		const QList<QPair<QString, QString>>& GetBrowser2ID () const;
		const QMap<QString, QString>& GetBackLookupMap () const;
	public slots:
		void initPlugin (QObject*);
		void hookUserAgentForUrlRequested (LC::IHookProxy_ptr, const QUrl&);
	};
}
}
}
