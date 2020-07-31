/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QHash>
#include <QPointer>
#include <interfaces/iinfo.h>
#include <interfaces/iplugin2.h>
#include <interfaces/ihavesettings.h>
#include <interfaces/core/ihookproxy.h>
#include <interfaces/azoth/iprovidecommands.h>

class QProcess;

namespace LC
{
namespace Azoth
{
class IProxyObject;

namespace SHX
{
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

		LC_PLUGIN_METADATA ("org.LeechCraft.Azoth.SHX")

		Util::XmlSettingsDialog_ptr XSD_;
		QHash<QProcess*, QPointer<QObject>> Process2Entry_;

		StaticCommand ExecCommand_;

		IProxyObject *AzothProxy_ = nullptr;
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
		void ExecuteProcess (ICLEntry *entry, const QString& text);
	public slots:
		void initPlugin (QObject*);
	private slots:
		void handleFinished ();
	};
}
}
}
