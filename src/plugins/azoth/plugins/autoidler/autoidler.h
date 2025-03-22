/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_AZOTH_PLUGINS_AUTOIDLER_AUTOIDLER_H
#define PLUGINS_AZOTH_PLUGINS_AUTOIDLER_AUTOIDLER_H
#include <memory>
#include <QObject>
#include <QMap>
#include <interfaces/iinfo.h>
#include <interfaces/iplugin2.h>
#include <interfaces/ihavesettings.h>
#include <interfaces/azoth/iclentry.h>
#include <interfaces/azoth/ilastactivityprovider.h>

class Idle;

namespace LC
{
namespace Azoth
{
class IProxyObject;

namespace Autoidler
{
	class Plugin : public QObject
				 , public IInfo
				 , public IPlugin2
				 , public IHaveSettings
				 , public ILastActivityProvider
	{
		Q_OBJECT
		Q_INTERFACES (IInfo IPlugin2 IHaveSettings LC::Azoth::ILastActivityProvider)

		LC_PLUGIN_METADATA ("org.LeechCraft.Azoth.AutoIdler")

		ICoreProxy_ptr Proxy_;
		IProxyObject *AzothProxy_;
		Util::XmlSettingsDialog_ptr XmlSettingsDialog_;

		std::shared_ptr<Idle> Idle_;

		QMap<QObject*, EntryStatus> OldStatuses_;

		int IdleSeconds_;
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

		int GetInactiveSeconds ();
	public slots:
		void initPlugin (QObject*);
	private slots:
		void handleIdle (int);
	};
}
}
}

#endif
