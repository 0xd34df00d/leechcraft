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

namespace LC::Lemon
{
	class TrafficManager;
	class ActionsManager;

	class PlatformBackend;
	using PlatformBackend_ptr = std::shared_ptr<PlatformBackend>;

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

		PlatformBackend_ptr Backend_;
	public:
		void Init (ICoreProxy_ptr) override;
		void SecondInit () override;
		QByteArray GetUniqueID () const override;
		void Release () override;
		QString GetName () const override;
		QString GetInfo () const override;
		QIcon GetIcon () const override;

		Util::XmlSettingsDialog_ptr GetSettingsDialog () const override;

		QuarkComponents_t GetComponents () const override;
	};
}
