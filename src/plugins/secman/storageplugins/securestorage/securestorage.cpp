/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2011  Alexander Konovalov
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

#include "securestorage.h"
#include <interfaces/secman/istorageplugin.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include <QSettings>
#include <QIcon>
#include <QCoreApplication>
#include <QInputDialog>
#include <QMessageBox>
#include <QVariant>
#include <QAction>
#include <QList>
#include <QMap>
#include <QDataStream>
#include <exception>
#include "settingswidget.h"
#include "xmlsettingsmanager.h"

namespace
{
	class PasswordNotEnteredException : std::exception
	{
	public:
		PasswordNotEnteredException () { }

		const char* what () const throw ()
		{
			return "PasswordNotEnteredException";
		}
	};

	QByteArray Serialize (const QVariant& variant)
	{
		QByteArray result;
		QDataStream stream (&result, QIODevice::ReadWrite);
		variant.save (stream);
		return result;
	}

	QVariant Deserialize (const QByteArray& array)
	{
		QVariant result;
		QDataStream stream (array);
		result.load (stream);
		return result;
	}
}

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
	Plugin::Plugin ()
	: WindowTitle_ ("SecMan SecureStorage")
	, CryptoSystem_ (0)
	{
	}

	void Plugin::Init (ICoreProxy_ptr)
	{
		Settings_ .reset (new QSettings (QSettings::IniFormat,
					QSettings::UserScope,
					QCoreApplication::organizationName (),
					QCoreApplication::applicationName () + "_SecMan_SecureStorage"));
		Storage_ .reset (new QSettings (QSettings::IniFormat,
					QSettings::UserScope,
					QCoreApplication::organizationName (),
					QCoreApplication::applicationName () + "_SecMan_SecureStorage_Data"));

		ForgetKeyAction_ = new QAction (tr ("Forget master password"), this);
		connect (ForgetKeyAction_, 
				SIGNAL (triggered ()),
				this, 
				SLOT (forgetKey ()));

		SettingsWidget_ = new SettingsWidget;
		connect (SettingsWidget_,
				SIGNAL (changePasswordRequested ()),
				this,
				SLOT (changePassword ()));
		connect (SettingsWidget_,
				SIGNAL (clearSettingsRequested ()),
				this,
				SLOT (clearSettings ()));

		XmlSettingsDialog_.reset (new Util::XmlSettingsDialog);
		XmlSettingsDialog_->RegisterObject (XmlSettingsManager::Instance (),
				"securestoragesettings.xml");
		XmlSettingsDialog_->SetCustomWidget ("SettingsWidget", SettingsWidget_);
		UpdateActionsStates ();
	}

	void Plugin::SecondInit ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.SecMan.StoragePlugins.SecureStorage";
	}

	void Plugin::Release ()
	{
		forgetKey ();
	}

	QString Plugin::GetName () const
	{
		return "SecureStorage";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Encrypted storage plugin for SecMan");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon ();
	}

	QSet<QByteArray> Plugin::GetPluginClasses () const
	{
		return QSet<QByteArray> () << "org.LeechCraft.SecMan.StoragePlugins/1.0";
	}

	QList<QAction*> Plugin::GetActions (ActionsEmbedPlace place) const
	{
		QList<QAction*> result;
		if (place == AEPCommonContextMenu || place == AEPTrayMenu)
			result << ForgetKeyAction_;
		return result;
	}

	IStoragePlugin::StorageTypes Plugin::GetStorageTypes () const
	{
		return STSecure;
	}

	void Plugin::UpdateActionsStates ()
	{
		ForgetKeyAction_->setEnabled (bool (CryptoSystem_));
	}

	QList<QByteArray> Plugin::ListKeys (IStoragePlugin::StorageType)
	{
		QList<QByteArray> result;
		Q_FOREACH (const QString& key, Storage_->allKeys ())
			result << key.toUtf8 ();
		return result;
	}

	void Plugin::Save (const QByteArray& key, const QVariantList& values,
			IStoragePlugin::StorageType st, bool overwrite)
	{
		QVariantList allValues = values;
		if (!overwrite)
			allValues.prepend (Load (key, st));

		try
		{
			const QByteArray& data = Serialize (allValues);
			const QByteArray& encrypted = GetCryptoSystem ().Encrypt (data);
			Storage_->setValue (key, encrypted);
		}
		catch (const PasswordNotEnteredException&)
		{
			qWarning () << Q_FUNC_INFO << "Password was not entered";
		}
		catch (const WrongHMACException&)
		{
			qWarning () << Q_FUNC_INFO << "Wrong HMAC";
			forgetKey ();
		}
	}

	QVariantList Plugin::Load (const QByteArray& key, IStoragePlugin::StorageType)
	{
		try
		{
			const QByteArray& encrypted = Storage_->value (key).toByteArray ();
			const QByteArray& data = GetCryptoSystem ().Decrypt (encrypted);
			return Deserialize (data).toList ();
		}
		catch (const WrongHMACException&)
		{
			return QVariantList ();
		}
		catch (const PasswordNotEnteredException&)
		{
			return QVariantList ();
		}
	}

	void Plugin::Save (const QList<QPair<QByteArray, QVariantList> >& keyValues,
			IStoragePlugin::StorageType st, bool overwrite)
	{
		QPair<QByteArray, QVariantList> keyValue;
		Q_FOREACH (keyValue, keyValues)
			Save (keyValue.first, keyValue.second, st, overwrite);
	}

	QList<QVariantList> Plugin::Load (const QList<QByteArray>& keys, IStoragePlugin::StorageType st)
	{
		QList<QVariantList> result;
		Q_FOREACH (const QByteArray& key, keys)
			result << Load (key, st);
		return result;
	}

	void Plugin::forgetKey ()
	{
		SetCryptoSystem (0);
	}

	void Plugin::clearSettings ()
	{
		// confirm
		QMessageBox::StandardButton r =
			QMessageBox::question (0,
					WindowTitle_,
					tr ("Do you really want to clear all stored data?"),
					QMessageBox::Yes | QMessageBox::No,
					QMessageBox::No);
		if (r != QMessageBox::Yes)
			return;

		SetCryptoSystem (0);
		Settings_->clear ();
		Storage_->clear ();
		UpdateActionsStates ();
	}

	void Plugin::changePassword ()
	{
		if (!IsPasswordSet ())
		{
			CreateNewPassword ();
			return;
		}

		// get old password from a settings
		QString oldPassword = SettingsWidget_->GetOldPassword ();
		CryptoSystem oldCs (oldPassword);
		if(!IsPasswordCorrect (oldCs))
		{
			QMessageBox::critical (0, WindowTitle_,
						tr ("Wrong old master password"),
						QMessageBox::Ok);
			return;
		}

		// get new password from a settings
		QString password = SettingsWidget_->GetNewPassword1 ();
		QString password2 = SettingsWidget_->GetNewPassword2 ();
		if (password != password2)
		{
			QMessageBox::critical (0,
						WindowTitle_,
						tr ("The passwords are different."),
						QMessageBox::Ok);
				return;
		}

		ChangePassword (oldPassword, password);
		
		// clear the password fields of the settings widget
		SettingsWidget_->ClearPasswordFields ();
	}

	const CryptoSystem& Plugin::GetCryptoSystem ()
	{
		if (!IsPasswordSet ())
			CreateNewPassword ();

		if (!CryptoSystem_)
		{
			if (IsPasswordEmpty ())
				SetCryptoSystem (new CryptoSystem (""));
			else
			{
				QInputDialog dialog;
				dialog.setTextEchoMode (QLineEdit::Password);
				dialog.setWindowTitle (WindowTitle_);
				dialog.setLabelText (tr ("Enter master password:"));
				while (true)
				{
					if (dialog.exec () != QDialog::Accepted)
						throw PasswordNotEnteredException ();

					QString password = dialog.textValue ();
					CryptoSystem *cs = new CryptoSystem (password);
					if (IsPasswordCorrect (*cs))
					{
						SetCryptoSystem (cs);
						break;
					}
					else // continue
						dialog.setLabelText (tr ("Wrong password.\n"
								"Try enter master password again:"));
				}
			}
		}
		return *CryptoSystem_;
	}

	void Plugin::SetCryptoSystem (CryptoSystem *cs)
	{
		delete CryptoSystem_;
		CryptoSystem_ = cs;
		UpdateActionsStates ();
	}

	void Plugin::CreateNewPassword ()
	{
		QInputDialog dialog;
		dialog.setWindowTitle (WindowTitle_);
		dialog.setTextEchoMode (QLineEdit::Password);

		QString password;
		while (true)
		{
			// query password
			dialog.setTextValue ("");
			dialog.setLabelText (tr ("Creating master password for SecureStorage.\n"
					"Enter new master password:"));
			if (dialog.exec () != QInputDialog::Accepted)
				break; // use empty password

			password = dialog.textValue ();

			// query password again
			dialog.setTextValue ("");
			dialog.setLabelText (tr ("Enter the same master password again:"));
			if (dialog.exec () != QInputDialog::Accepted)
				break; // use empty password

			QString password2 = dialog.textValue ();
			if (password == password2)
				break; // the "right" way

			else if (QMessageBox::question (0,
						WindowTitle_,
						tr ("Two passwords that you entered are not equal.\n"
							"Do you want to try again?"),
						QMessageBox::Yes | QMessageBox::No,
						QMessageBox::Yes) != QMessageBox::Yes)
				throw PasswordNotEnteredException ();
			// else continue;
		}

		// clear old settings and data
		Settings_->clear ();
		Storage_->clear ();

		// set up new settings
		UpdatePasswordSettings (password);
		UpdateActionsStates ();
	}

	bool Plugin::IsPasswordSet ()
	{
		return Settings_->value ("SecureStoragePasswordIsSet").toBool ();
	}

	void Plugin::ChangePassword (const QString& oldPass, const QString& newPass)
	{
		CryptoSystem oldCs (oldPass);
		if (!IsPasswordCorrect (oldCs))
		{
			qWarning () << Q_FUNC_INFO << "Called with incorrect old password";
			return;
		}

		CryptoSystem newCs (newPass);

		Q_FOREACH (const QString& key, Storage_->allKeys ())
		{
			try
			{
				const QByteArray& oldEncrypted = Storage_->value (key).toByteArray ();
				const QByteArray& data = oldCs.Decrypt (oldEncrypted);
				const QByteArray& newEncrypted = newCs.Encrypt (data);
				QVariant encryptedData (newEncrypted);
				Storage_->setValue (key, encryptedData);
			}
			catch (const WrongHMACException&)
			{
				qWarning () << Q_FUNC_INFO <<
						"Removing value of key \"" << key << "\" (wrong HMAC)";
				Storage_->remove (key);
			}
		}

		UpdatePasswordSettings (newPass);
		UpdateActionsStates ();
	}

	void Plugin::UpdatePasswordSettings (const QString& pass)
	{
		CryptoSystem *cs = new CryptoSystem (pass);

		// set up new settings
		Settings_->setValue ("SecureStoragePasswordIsSet", true);
		Settings_->setValue ("SecureStoragePasswordIsEmpty", pass.isEmpty ());
		const QByteArray& cookie = cs->Encrypt (QByteArray ("cookie"));
		Settings_->setValue ("SecureStorageCookie", cookie);

		// use created cryptosystem.
		SetCryptoSystem (cs);
	}

	bool Plugin::IsPasswordCorrect (const CryptoSystem& cs)
	{
		if (!IsPasswordSet ())
			return false;

		const QByteArray& cookie = Settings_->value ("SecureStorageCookie").toByteArray ();
		try
		{
			cs.Decrypt (cookie);
			return true;
		}
		catch (const WrongHMACException&)
		{
			return false;
		}
	}

	bool Plugin::IsPasswordEmpty ()
	{
		return Settings_->value ("SecureStoragePasswordIsEmpty").toBool ();
	}

	LeechCraft::Util::XmlSettingsDialog_ptr Plugin::GetSettingsDialog () const
	{
		return XmlSettingsDialog_;
	}
}
}
}
}
}

Q_EXPORT_PLUGIN2 (leechcraft_secman_securestorage, LeechCraft::Plugins::SecMan::StoragePlugins::SecureStorage::Plugin);
