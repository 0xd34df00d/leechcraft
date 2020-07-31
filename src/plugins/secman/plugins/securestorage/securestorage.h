/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2011-2012  Alexander Konovalov
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QObject>
#include <QInputDialog>
#include <interfaces/iinfo.h>
#include <interfaces/iplugin2.h>
#include <interfaces/secman/istorageplugin.h>
#include <interfaces/iactionsexporter.h>
#include <interfaces/ihavesettings.h>
#include "cryptosystem.h"
#include "settingswidget.h"
#include "newpassworddialog.h"

class QSettings;

namespace LC
{
namespace SecMan
{
namespace SecureStorage
{
	class Plugin : public QObject
				 , public IInfo
				 , public IPlugin2
				 , public IActionsExporter
				 , public IStoragePlugin
				 , public IHaveSettings
	{
		Q_OBJECT
		Q_INTERFACES (IInfo IPlugin2 LC::SecMan::IStoragePlugin IActionsExporter IHaveSettings)

		LC_PLUGIN_METADATA ("org.LeechCraft.SecMan.SecureStorage")

		Util::XmlSettingsDialog_ptr XmlSettingsDialog_;
		SettingsWidget* SettingsWidget_;

		const std::shared_ptr<QSettings> Storage_;
		const std::shared_ptr<QSettings> Settings_;

		QString WindowTitle_;
		CryptoSystem_ptr CryptoSystem_;

		QAction *ForgetKeyAction_;
		QAction *InputKeyAction_;

		std::shared_ptr<QInputDialog> InputPasswordDialog_;
		std::shared_ptr<NewPasswordDialog> NewPasswordDialog_;
	public:
		Plugin ();
		void Init (ICoreProxy_ptr);
		void SecondInit ();
		QByteArray GetUniqueID () const;
		void Release ();
		QString GetName () const;
		QString GetInfo () const;
		QIcon GetIcon () const;

		QSet<QByteArray> GetPluginClasses () const;

		StorageTypes GetStorageTypes () const;
		QList<QByteArray> ListKeys (StorageType);
		void Save (const QByteArray&, const QVariant&, StorageType);
		QVariant Load (const QByteArray&, StorageType);
		QList<QAction*> GetActions (LC::ActionsEmbedPlace) const;

		Util::XmlSettingsDialog_ptr GetSettingsDialog () const;
	public slots:
		void forgetKey ();
		void inputKey ();
		void changePassword ();
		void clearSettings ();
	private:
		const CryptoSystem& GetCryptoSystem ();
		void SetCryptoSystem (const CryptoSystem_ptr&);
		void UpdateActionsStates ();
		void UpdatePasswordSettings (const QString& pass);

		void ChangePassword (const QString& oldPass, const QString& newPass);
		void CreateNewPassword ();
		bool IsPasswordCorrect (const CryptoSystem& cs);
		bool IsPasswordEmpty ();
		bool IsPasswordSet ();
	signals:
		void gotActions (QList<QAction*>, ActionsEmbedPlace);
	};

	class PasswordNotEnteredException : std::exception
	{
	public:
		PasswordNotEnteredException () { }

		const char* what () const throw ()
		{
			return "PasswordNotEnteredException";
		}
	};

	/// return s1 if s1==s2, else throw PasswordNotEnteredException.
	QString ReturnIfEqual (const QString& s1, const QString& s2);
}
}
}
