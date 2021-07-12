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
#include <interfaces/core/ihookproxy.h>
#include <interfaces/azoth/iprovidecommands.h>

namespace Media
{
	struct AudioInfo;
	class ICurrentSongKeeper;
}

namespace LC
{
namespace Azoth
{
class IProxyObject;

namespace Xtazy
{
	class TuneSourceBase;

	class Plugin : public QObject
				 , public IInfo
				 , public IPlugin2
				 , public IHaveSettings
				 , public IProvideCommands
	{
		Q_OBJECT
		Q_INTERFACES (IInfo
				IPlugin2
				IHaveSettings
				LC::Azoth::IProvideCommands)

		LC_PLUGIN_METADATA ("org.LeechCraft.Azoth.Xtazy")

		IProxyObject *AzothProxy_ = nullptr;
		ICoreProxy_ptr Proxy_;

		Media::ICurrentSongKeeper *Keeper_ = nullptr;

		Util::XmlSettingsDialog_ptr XSD_;

		typedef QPair<QPointer<QObject>, QString> UploadNotifee_t;
		QMap<QString, QList<UploadNotifee_t>> PendingUploads_;

		StaticCommands_t Commands_;
	public:
		void Init (ICoreProxy_ptr);
		void SecondInit ();
		QByteArray GetUniqueID () const;
		void Release ();
		QString GetName () const;
		QString GetInfo () const;
		QIcon GetIcon () const;

		QSet<QByteArray> GetPluginClasses () const;

		Util::XmlSettingsDialog_ptr GetSettingsDialog () const;

		StaticCommands_t GetStaticCommands (ICLEntry*);
	private:
		bool SendCurrentSong (ICLEntry*, QString&);
		bool HandleShare (ICLEntry*, QString&);

		void SendAudioInfo (const Media::AudioInfo&);
	public slots:
		void initPlugin (QObject*);
	private slots:
		void publish (const Media::AudioInfo&);
		void handleFileUploaded (const QString&, const QUrl&);

		void handleAutoPublishChanged ();
	};
}
}
}
