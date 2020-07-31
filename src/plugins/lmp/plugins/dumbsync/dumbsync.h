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
#include <interfaces/ihavesettings.h>
#include <interfaces/iplugin2.h>
#include <interfaces/lmp/ilmpplugin.h>
#include <interfaces/lmp/isyncplugin.h>

namespace LC
{
namespace LMP
{
namespace DumbSync
{
	class Plugin : public QObject
				 , public IInfo
				 , public IHaveSettings
				 , public IPlugin2
				 , public ILMPPlugin
				 , public ISyncPlugin
	{
		Q_OBJECT
		Q_INTERFACES (IInfo
				IHaveSettings
				IPlugin2
				LC::LMP::ILMPPlugin
				LC::LMP::ISyncPlugin)

		LC_PLUGIN_METADATA ("org.LeechCraft.LMP.DumbSync")

		Util::XmlSettingsDialog_ptr XSD_;
		ILMPProxy_ptr LMPProxy_;
	public:
		void Init (ICoreProxy_ptr proxy);
		void SecondInit ();
		void Release ();
		QByteArray GetUniqueID () const;
		QString GetName () const;
		QString GetInfo () const;
		QIcon GetIcon () const;

		Util::XmlSettingsDialog_ptr GetSettingsDialog () const;

		QSet<QByteArray> GetPluginClasses () const;

		void SetLMPProxy (ILMPProxy_ptr);

		QObject* GetQObject ();
		QString GetSyncSystemName () const;
		SyncConfLevel CouldSync (const QString&);
		void Upload (const QString& localPath, const QString& origLocalPath,
				const QString& to, const QString& relPath);
	private slots:
		void handleCopyFinished ();
	signals:
		void uploadFinished (const QString&, QFile::FileError, const QString&);
	};
}
}
}
