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
#include <interfaces/itexteditor.h>
#include <interfaces/ihavesettings.h>

namespace LC::LHTR
{
	class Plugin : public QObject
				 , public IInfo
				 , public ITextEditor
				 , public IHaveSettings
	{
		Q_OBJECT
		Q_INTERFACES (IInfo ITextEditor IHaveSettings)

		LC_PLUGIN_METADATA ("org.LeechCraft.LHTR")

		Util::XmlSettingsDialog_ptr XSD_;
	public:
		void Init (ICoreProxy_ptr) override;
		void SecondInit () override;
		QByteArray GetUniqueID () const override;
		void Release () override;
		QString GetName () const override;
		QString GetInfo () const override;
		QIcon GetIcon () const override;

		bool SupportsEditor (ContentType) const override;
		QWidget* GetTextEditor (ContentType) override;

		Util::XmlSettingsDialog_ptr GetSettingsDialog () const override;
	};
}
