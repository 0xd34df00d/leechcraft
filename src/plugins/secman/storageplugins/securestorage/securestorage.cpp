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
#include <QSettings>
#include <QIcon>
#include <QCoreApplication>
#include <QInputDialog>
#include <QVariant>
#include <QDataStream>
#include <openssl/aes.h>
#include <openssl/sha.h>
#include <openssl/rand.h>
#include <openssl/crypto.h>
#include <openssl/hmac.h>
#include <exception>

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

					class PasswordNotEnteredException : std::exception
					{
					public:

						PasswordNotEnteredException ()
						{
						}
					};

					void Plugin::Init (ICoreProxy_ptr)
					{
						CryptoSystem_ = 0;
						Storage_ .reset (new QSettings (QSettings::IniFormat,
								QSettings::UserScope,
								QCoreApplication::organizationName (),
								QCoreApplication::applicationName () + "_SecMan_SecureStorage"));
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
						return STInsecure;
					}

					inline bool IsDataKey (const QString &keyName)
					{
						return keyName.startsWith ("d");
					}

					inline bool IsMetaDataKey (const QString &keyName)
					{
						return keyName.startsWith ("m");
					}

					inline QString ToDataKey (const QString &key)
					{
						return QString ("d") + key;
					}

					inline QString ToMetaDataKey (const QString &key)
					{
						return QString ("m") + key;
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

					const CryptoSystem &Plugin::GetCryptoSystem ()
					{
						if (!CryptoSystem_)
						{
							QInputDialog dialog;
							dialog.setTextEchoMode (QLineEdit::Password);
							dialog.setWindowTitle (tr ("SecMan SecureStorage"));
							dialog.setLabelText (tr ("Enter master password:"));
							QString password ("");

							if (dialog.exec () == QDialog::Accepted)
								password = dialog.textValue ();
							CryptoSystem_ = new CryptoSystem (password);
						}
						return *CryptoSystem_;
					}

					void Plugin::forgetKey ()
					{

						delete CryptoSystem_;
						CryptoSystem_ = 0;
					}

					void Plugin::changePassword (const QString &oldPass, const QString &newPass)
					{
					}
				}
			}
		}
	}
}

Q_EXPORT_PLUGIN2 (leechcraft_secman_securestorage, LeechCraft::Plugins::SecMan::StoragePlugins::SecureStorage::Plugin);
