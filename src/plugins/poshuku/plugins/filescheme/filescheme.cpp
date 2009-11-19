/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
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

#include "filescheme.h"
#include <typeinfo>
#include <boost/bind.hpp>
#include <plugininterface/util.h>
#include "schemereply.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Poshuku
		{
			namespace Plugins
			{
				namespace FileScheme
				{
					void FileScheme::Init (ICoreProxy_ptr proxy)
					{
						Translator_.reset (Util::InstallTranslator ("poshuku_filescheme"));

						proxy->RegisterHook (HookSignature<HIDNetworkAccessManagerCreateRequest>::Signature_t (
									boost::bind (&FileScheme::CreateRequest,
										this,
										_1,
										_2,
										_3,
										_4,
										_5)));
					}

					void FileScheme::SecondInit ()
					{
					}

					void FileScheme::Release ()
					{
					}

					QString FileScheme::GetName () const
					{
						return "Poshuku FileScheme";
					}

					QString FileScheme::GetInfo () const
					{
						return tr ("Provides support for file:// scheme.");
					}

					QIcon FileScheme::GetIcon () const
					{
						return QIcon (":/plugins/poshuku/plugins/filescheme/resources/images/poshuku_filescheme.svg");
					}

					QStringList FileScheme::Provides () const
					{
						return QStringList ("file://");
					}

					QStringList FileScheme::Needs () const
					{
						return QStringList ();
					}

					QStringList FileScheme::Uses () const
					{
						return QStringList ();
					}

					void FileScheme::SetProvider (QObject*, const QString&)
					{
					}

					QByteArray FileScheme::GetPluginClass () const
					{
						return QByteArray (typeid (PluginBase).name ());
					}

					void FileScheme::Init (IProxyObject*)
					{
					}

					QNetworkReply* FileScheme::CreateRequest (IHookProxy_ptr proxy,
							QNetworkAccessManager*,
							QNetworkAccessManager::Operation *op,
							const QNetworkRequest *req,
							QIODevice **)
					{
						if (*op != QNetworkAccessManager::GetOperation)
							return 0;

						QString path = req->url ().toLocalFile ();

						if (!QFileInfo (path).isDir ())
							return 0;

						proxy->CancelDefault ();
						return new SchemeReply (*req, this);
					}
				};
			};
		};
	};
};

Q_EXPORT_PLUGIN2 (leechcraft_poshuku_filescheme,
		LeechCraft::Plugins::Poshuku::Plugins::FileScheme::FileScheme);

