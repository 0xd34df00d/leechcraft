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

#include "securestorage.h"
#include <exception>
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
#include <interfaces/secman/istorageplugin.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include "settingswidget.h"
#include "xmlsettingsmanager.h"

namespace
{
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

	QString ReturnIfEqual (const QString& s1, const QString& s2)
	{
		if (s1 == s2)
			return s1;
		else
			throw PasswordNotEnteredException ();
	}

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

		InputKeyAction_ = new QAction (tr ("Enter master password..."), this);
		connect (InputKeyAction_,
				SIGNAL (triggered ()),
				this,
				SLOT (inputKey ()));

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

		InputPasswordDialog_.reset (new QInputDialog);
		NewPasswordDialog_.reset (new NewPasswordDialog);
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
		if (place == ActionsEmbedPlace::CommonContextMenu
				|| place == ActionsEmbedPlace::TrayMenu)
			result << ForgetKeyAction_ << InputKeyAction_;
		return result;
	}

	IStoragePlugin::StorageTypes Plugin::GetStorageTypes () const
	{
		return STSecure;
	}

	void Plugin::UpdateActionsStates ()
	{
		ForgetKeyAction_->setEnabled (CryptoSystem_ && !IsPasswordEmpty ());
		InputKeyAction_->setEnabled (!CryptoSystem_ && !IsPasswordEmpty ());
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

	void Plugin::Save (const QList<QPair<QByteArray, QVariantList>>& keyValues,
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

	void Plugin::inputKey ()
	{
		GetCryptoSystem ();
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
		if (!IsPasswordCorrect (oldCs))
		{
			QMessageBox::critical (0, WindowTitle_,
						tr ("Wrong old master password"),
						QMessageBox::Ok);
			return;
		}

		// get new password from a settings
		try
		{
			QString password = SettingsWidget_->GetNewPassword ();
			ChangePassword (oldPassword, password);
			// clear the password fields of the settings widget
			SettingsWidget_->ClearPasswordFields ();
		}
		catch (const PasswordNotEnteredException&)
		{
			QMessageBox::critical (0,
						WindowTitle_,
						tr ("The passwords are different."),
						QMessageBox::Ok);
		}
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
				while (true)
				{
					// This method can be called recursively from loop.exec() below,
					// but we should display only one password dialog.
					if (!InputPasswordDialog_->isVisible ())
					{
						InputPasswordDialog_->setTextEchoMode (QLineEdit::Password);
						InputPasswordDialog_->setWindowTitle (WindowTitle_);
						InputPasswordDialog_->setLabelText (tr ("Enter master password:"));
						InputPasswordDialog_->setTextValue (QString ());
						InputPasswordDialog_->setVisible (true);
					}

					QEventLoop loop;
					connect (InputPasswordDialog_.get (),
						SIGNAL (accepted ()),
						&loop,
						SLOT (quit ()),
						Qt::QueuedConnection);
					connect (InputPasswordDialog_.get (),
						SIGNAL (rejected ()),
						&loop,
						SLOT (quit ()),
						Qt::QueuedConnection);
					// qDebug () << Q_FUNC_INFO << "Loop start";
					loop.exec ();
					// qDebug () << Q_FUNC_INFO << "Loop exit";

					if (CryptoSystem_)
						break;

					if (InputPasswordDialog_->result () != QDialog::Accepted)
						throw PasswordNotEnteredException ();

					QString password = InputPasswordDialog_->textValue ();
					CryptoSystem *cs = new CryptoSystem (password);
					if (IsPasswordCorrect (*cs))
					{
						SetCryptoSystem (cs);
						break;
					}
					else // continue
					{
						delete cs;
						InputPasswordDialog_->setLabelText (tr ("Wrong password.\n"
								"Try enter master password again:"));
					}
				}
			}
		}
		// qDebug () << Q_FUNC_INFO << "ok";
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
		if (!NewPasswordDialog_->isVisible ())
		{
			NewPasswordDialog_->clear ();
			NewPasswordDialog_->show ();
		}
		QEventLoop loop;
		connect (NewPasswordDialog_.get (),
			SIGNAL (dialogFinished ()),
			&loop,
			SLOT (quit ()),
			Qt::QueuedConnection);
		// qDebug () << Q_FUNC_INFO << "Loop start";
		loop.exec ();
		// qDebug () << Q_FUNC_INFO << "Loop exit";

		QString password = NewPasswordDialog_->GetPassword ();

		// check result of recursive calls of this method through loop.exec().
		if (!IsPasswordSet ())
		{
			// clear old settings and data
			Settings_->clear ();
			Storage_->clear ();

			// set up new settings
			UpdatePasswordSettings (password);
			UpdateActionsStates ();
		}
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

LC_EXPORT_PLUGIN (leechcraft_secman_securestorage, LeechCraft::Plugins::SecMan::StoragePlugins::SecureStorage::Plugin);
