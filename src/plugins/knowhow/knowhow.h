/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_KNOWHOW_KNOWHOW_H
#define PLUGINS_KNOWHOW_KNOWHOW_H
#include <QObject>
#include <interfaces/iinfo.h>
#include <interfaces/ihavesettings.h>

namespace LC
{
namespace KnowHow
{
	class Plugin : public QObject
				 , public IInfo
				 , public IHaveSettings
	{
		Q_OBJECT
		Q_INTERFACES (IInfo IHaveSettings)

		LC_PLUGIN_METADATA ("org.LeechCraft.KnowHow")

		ICoreProxy_ptr Proxy_;
		Util::XmlSettingsDialog_ptr SettingsDialog_;
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
		void showTip ();
	};
}
}

#endif

