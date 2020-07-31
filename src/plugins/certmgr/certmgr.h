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
#include <interfaces/iinfo.h>
#include <interfaces/ihavesettings.h>
#include "manager.h"

namespace LC
{
namespace CertMgr
{
	class Plugin : public QObject
				 , public IInfo
				 , public IHaveSettings
	{
		Q_OBJECT
		Q_INTERFACES (IInfo IHaveSettings)

		LC_PLUGIN_METADATA ("org.LeechCraft.CertMgr")

		ICoreProxy_ptr Proxy_;
		Util::XmlSettingsDialog_ptr XSD_;

		std::unique_ptr<Manager> Manager_;
	public:
		void Init (ICoreProxy_ptr);
		void SecondInit ();
		QByteArray GetUniqueID () const;
		void Release ();
		QString GetName () const;
		QString GetInfo () const;
		QIcon GetIcon () const;

		Util::XmlSettingsDialog_ptr GetSettingsDialog () const;
	private slots:
		void handleSettingsButton (const QString&);
	};
}
}
