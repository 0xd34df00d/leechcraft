/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <interfaces/iinfo.h>
#include <interfaces/iplugin2.h>
#include <interfaces/ihavesettings.h>
#include <interfaces/ihavediaginfo.h>
#include <interfaces/monocle/ibackendplugin.h>
#include <interfaces/monocle/iknowfileextensions.h>

namespace LC::Monocle::PDF
{
	class Plugin : public QObject
				 , public IInfo
				 , public IPlugin2
				 , public IHaveSettings
				 , public IHaveDiagInfo
				 , public IBackendPlugin
				 , public IKnowFileExtensions
	{
		Q_OBJECT
		Q_INTERFACES (IInfo
				IPlugin2
				IHaveSettings
				IHaveDiagInfo
				LC::Monocle::IBackendPlugin
				LC::Monocle::IKnowFileExtensions)

		LC_PLUGIN_METADATA ("org.LeechCraft.Monocle.PDF")

		Util::XmlSettingsDialog_ptr XSD_;
	public:
		void Init (ICoreProxy_ptr) override;
		void SecondInit () override;
		QByteArray GetUniqueID () const override;
		void Release () override;
		QString GetName () const override;
		QString GetInfo () const override;
		QIcon GetIcon () const override;

		QSet<QByteArray> GetPluginClasses () const override;

		Util::XmlSettingsDialog_ptr GetSettingsDialog () const override;

		QString GetDiagInfoString () const override;

		LoadCheckResult CanLoadDocument (const QString&) override;
		IDocument_ptr LoadDocument (const QString&) override;

		QStringList GetSupportedMimes () const override;
		bool IsThreaded () const override;

		QList<ExtInfo> GetKnownFileExtensions () const override;
	};
}
