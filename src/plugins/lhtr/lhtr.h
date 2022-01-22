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

namespace LC
{
namespace LHTR
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
		void Init (ICoreProxy_ptr);
		void SecondInit ();
		QByteArray GetUniqueID () const;
		void Release ();
		QString GetName () const;
		QString GetInfo () const;
		QIcon GetIcon () const;

		bool SupportsEditor (ContentType) const;
		QWidget* GetTextEditor (ContentType);

		Util::XmlSettingsDialog_ptr GetSettingsDialog () const;
	};
}
}
