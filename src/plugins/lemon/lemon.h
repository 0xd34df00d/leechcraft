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
#include <interfaces/iquarkcomponentprovider.h>

namespace LC
{
namespace Lemon
{
	class TrafficManager;
	class ActionsManager;

	class Plugin : public QObject
				 , public IInfo
				 , public IHaveSettings
				 , public IQuarkComponentProvider
	{
		Q_OBJECT
		Q_INTERFACES (IInfo IHaveSettings IQuarkComponentProvider)

		LC_PLUGIN_METADATA ("org.LeechCraft.Lemon")

		Util::XmlSettingsDialog_ptr XSD_;

		TrafficManager *TrafficMgr_;
		QuarkComponent_ptr PanelComponent_;
	public:
		void Init (ICoreProxy_ptr);
		void SecondInit ();
		QByteArray GetUniqueID () const;
		void Release ();
		QString GetName () const;
		QString GetInfo () const;
		QIcon GetIcon () const;

		Util::XmlSettingsDialog_ptr GetSettingsDialog () const;

		QuarkComponents_t GetComponents () const;
	};
}
}
