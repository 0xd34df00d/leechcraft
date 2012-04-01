/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2011-2012  Alexander Konovalov
 * Copyright (C) 2006-2012  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#ifndef PLUGINS_SECMAN_PLUGINS_SECURESTORAGE_SECURESTORAGE_H
#define PLUGINS_SECMAN_PLUGINS_SECURESTORAGE_SECURESTORAGE_H
#include <boost/shared_ptr.hpp>
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

namespace LeechCraft
{
namespace Plugins
{
namespace SecMan
{
namespace StoragePlugins
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
		Q_INTERFACES (IInfo IPlugin2 LeechCraft::Plugins::SecMan::IStoragePlugin IActionsExporter IHaveSettings)

		Util::XmlSettingsDialog_ptr XmlSettingsDialog_;
		SettingsWidget* SettingsWidget_;
		
		boost::shared_ptr<QSettings> Storage_;
		boost::shared_ptr<QSettings> Settings_;

		QString WindowTitle_;
		CryptoSystem *CryptoSystem_;

		QAction *ForgetKeyAction_;
		QAction *InputKeyAction_;

		boost::shared_ptr<QInputDialog> InputPasswordDialog_;
		boost::shared_ptr<NewPasswordDialog> NewPasswordDialog_;
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
		void Save (const QByteArray&, const QVariantList&, StorageType, bool);
		QVariantList Load (const QByteArray&, StorageType);
		void Save (const QList<QPair<QByteArray, QVariantList>>&, StorageType, bool);
		QList<QVariantList> Load (const QList<QByteArray>&, StorageType);
		QList<QAction*> GetActions (LeechCraft::ActionsEmbedPlace) const;
		
		LeechCraft::Util::XmlSettingsDialog_ptr GetSettingsDialog () const;
	public slots:
		void forgetKey ();
		void inputKey ();
		void changePassword ();
		void clearSettings ();
	private:
		const CryptoSystem& GetCryptoSystem ();
		void SetCryptoSystem (CryptoSystem *cs);
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
}
}

#endif
