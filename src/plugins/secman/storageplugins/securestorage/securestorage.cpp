/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2011  Alexander Konovalov
 * Copyright (C) 2006-2011  Georg Rudoy
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
#include <QSettings>
#include <QIcon>
#include <QCoreApplication>
#include <QInputDialog>
#include <QMessageBox>
#include <QVariant>
#include <QDataStream>
#include <exception>

using namespace LeechCraft::Plugins::SecMan::StoragePlugins::SecureStorage;
using namespace LeechCraft::Plugins::SecMan;

namespace
{

	class PasswordNotEnteredException : std::exception
	{
	public:

		PasswordNotEnteredException ()
		{
		}
	};
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

QStringList Plugin::Provides () const
{
	return QStringList ();
}

QStringList Plugin::Needs () const
{
	return QStringList ();
}

QStringList Plugin::Uses () const
{
	return QStringList ();
}

void Plugin::SetProvider (QObject*, const QString&)
{
}

QSet<QByteArray> Plugin::GetPluginClasses () const
{
	return QSet<QByteArray > () << "org.LeechCraft.SecMan.StoragePlugins/1.0";
}

IStoragePlugin::StorageTypes Plugin::GetStorageTypes () const
{
	return STSecure;
}

QByteArray serialize (const QVariant &variant)
{
	QByteArray result;
	QDataStream stream (&result, QIODevice::ReadWrite);
	variant.save (stream);
	return result;
}

QVariant deserialize (const QByteArray &array)
{
	QVariant result;
	QDataStream stream (array);
	result.load (stream);
	return result;
}

QList<QByteArray> Plugin::ListKeys (IStoragePlugin::StorageType st)
{
	QStringList keys = Storage_->childKeys ();
	QList<QByteArray> result;
	Q_FOREACH (const QString& key, keys)
	result << key.toUtf8 ();
	return result;
}

void Plugin::Save (const QByteArray& key, const QVariantList& values,
		IStoragePlugin::StorageType st, bool overwrite)
{
	QVariantList allValues;
	if (overwrite)
		allValues = values;
	else
		allValues = Load (key, st) + values;
	QByteArray encrypted = GetCryptoSystem ().Encrypt (serialize (allValues));
	Storage_->setValue (key, encrypted);
}

QVariantList Plugin::Load (const QByteArray& key, IStoragePlugin::StorageType st)
{
	try
	{
		QByteArray encrypted = Storage_->value (key).toByteArray ();
		return deserialize (GetCryptoSystem ().Decrypt (encrypted)).toList ();
	}
	catch (WrongHMACException&e)
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
	delete CryptoSystem_;
	CryptoSystem_ = 0;
}

void Plugin::clearSettings ()
{
	// confirm
	QMessageBox::StandardButton r =
			QMessageBox::question (0, WindowTitle_,
			tr ("Do you really want to clear all stored data?"),
			QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
	if (r != QMessageBox::Yes)
		return;

	Settings_->clear ();
	Storage_->clear ();
}

void Plugin::changePassword ()
{
	qWarning () << Q_FUNC_INFO << "Not implemented";
}

const CryptoSystem &Plugin::GetCryptoSystem ()
{
	if (!CryptoSystem_)
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
			CryptoSystem* cs = new CryptoSystem (password);
			if (IsPasswordCorrect (*cs))
			{
				CryptoSystem_ = cs;
				break;
			}
			else
				dialog.setLabelText (tr ("Wrong password.\n"
					"Try enter master password again:"));
		}
	}
	return *CryptoSystem_;
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
		dialog.setLabelText (tr ("Enter new master password"));
		if (dialog.exec () != QInputDialog::Accepted)
			return;
		password = dialog.textValue ();
		// query password again
		dialog.setLabelText (tr ("Enter same master password again"));
		if (dialog.exec () != QInputDialog::Accepted)
			return;
		QString password2 = dialog.textValue ();
		if (password == password2)
			break; // the "right" way
		else if (QMessageBox::question (0, WindowTitle_,
				tr ("Two passwords that you entered are not equal.\n"
				"Do you want to try again?"),
				QMessageBox::Yes | QMessageBox::No,
				QMessageBox::Yes) != QMessageBox::Yes)
			return;
		// else continue;
	}
	// clear old settings and data
	Settings_->clear ();
	Storage_->clear ();
	// set up new settings
	CryptoSystem* cs = new CryptoSystem (password);
	Settings_->setValue ("SecureStoragePasswordIsSet", QVariant (true));
	QByteArray cookie = cs->Encrypt (QByteArray ("cookie"));
	Settings_->setValue ("SecureStorageCookie", cookie);
	// use created cryptosystem.
	delete CryptoSystem_;
	CryptoSystem_ = cs;
}

bool Plugin::IsPasswordSet ()
{
	return Settings_->value ("SecureStoragePasswordIsSet").toBool ();
}

void Plugin::ChangePassword (const QString &oldPass, const QString &newPass)
{
	qWarning () << Q_FUNC_INFO << "Not implemented";
}

bool Plugin::IsPasswordCorrect (const CryptoSystem &cs)
{
	if (!IsPasswordSet ())
		return false;

	QByteArray cookie = Settings_->value ("SecureStorageCookie").toByteArray ();
	try
	{
		cs.Decrypt (cookie);
		return true;
	}
	catch (WrongHMACException &e)
	{
		return false;
	}
}

Q_EXPORT_PLUGIN2 (leechcraft_secman_securestorage, LeechCraft::Plugins::SecMan::StoragePlugins::SecureStorage::Plugin);
