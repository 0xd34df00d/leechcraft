/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <interfaces/iinfo.h>
#include <interfaces/ihavesettings.h>
#include <interfaces/iquarkcomponentprovider.h>
#include <interfaces/core/icoretabwidget.h>

namespace LC
{
namespace KBSwitch
{
	class KeyboardLayoutSwitcher;

	class Plugin : public QObject
				 , public IInfo
				 , public IHaveSettings
				 , public IQuarkComponentProvider
	{
		Q_OBJECT
		Q_INTERFACES (IInfo IHaveSettings IQuarkComponentProvider)

		LC_PLUGIN_METADATA ("org.LeechCraft.KBSwitch")

		ICoreProxy_ptr Proxy_;
		Util::XmlSettingsDialog_ptr SettingsDialog_;
		KeyboardLayoutSwitcher *KBLayoutSwitcher_;

		QuarkComponent_ptr Indicator_;
	public:
		void Init (ICoreProxy_ptr proxy);
		void SecondInit ();
		QByteArray GetUniqueID () const;
		void Release ();
		QString GetName () const;
		QString GetInfo () const;
		QIcon GetIcon () const;

		Util::XmlSettingsDialog_ptr GetSettingsDialog () const;

		QuarkComponents_t GetComponents () const;
	private slots:
		void handleCurrentChanged (int index);
		void handleCurrentWindowChanged (int from, int to);
		void handleWindow (int);
	};
}
}
