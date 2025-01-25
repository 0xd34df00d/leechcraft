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
#include <interfaces/ispellcheckprovider.h>

namespace LC
{
namespace Rosenthal
{
	class KnownDictsManager;

	class Plugin : public QObject
				 , public IInfo
				 , public IHaveSettings
				 , public ISpellCheckProvider
	{
		Q_OBJECT
		Q_INTERFACES (IInfo IHaveSettings ISpellCheckProvider)

		LC_PLUGIN_METADATA ("org.LeechCraft.Rosenthal")

		ICoreProxy_ptr Proxy_;

		Util::XmlSettingsDialog_ptr SettingsDialog_;
		KnownDictsManager *KnownMgr_ = nullptr;
	public:
		void Init (ICoreProxy_ptr) override;
		void SecondInit () override;
		QByteArray GetUniqueID () const override;
		void Release () override;
		QString GetName () const override;
		QString GetInfo () const override;
		QIcon GetIcon () const override;

		Util::XmlSettingsDialog_ptr GetSettingsDialog () const override;

		ISpellChecker_ptr CreateSpellchecker () override;
	private slots:
		void handlePushButtonClicked (const QString&);
	};
}
}
