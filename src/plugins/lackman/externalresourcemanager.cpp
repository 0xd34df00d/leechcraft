/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2010  Georg Rudoy
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

#include "externalresourcemanager.h"
#include <stdexcept>
#include <QtDebug>
#include <plugininterface/util.h>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace LackMan
		{
			namespace
			{
				QString URLToFileName (const QUrl& url)
				{
					return QString (url.toEncoded ().toBase64 ().replace ('/', '_'));
				}

				QDir GetDir ()
				{
					return Util::CreateIfNotExists ("lackman/resources/");
				}
			}

			ExternalResourceManager::ExternalResourceManager (QObject *parent)
			: QObject (parent)
			, ResourcesDir_ (GetDir ())
			{
			}

			boost::optional<QByteArray> ExternalResourceManager::GetResourceData (const QUrl& url)
			{
				QString fileName = URLToFileName (url);

				bool hasFile = ResourcesDir_.entryList ().contains (fileName);
				if (hasFile &&
						ResourcesDir_.entryInfoList (QStringList (fileName)).at (0).size ())
				{
					QString path = ResourcesDir_.filePath (fileName);
					QFile file (path);
					if (!file.open (QIODevice::ReadOnly))
					{
						QString errorString = QString ("Could not open "
									"file for reading %1: %2.")
								.arg (path)
								.arg (file.errorString ());
						qWarning () << Q_FUNC_INFO
								<< errorString;
						throw std::runtime_error (qPrintable (errorString));
					}
					return file.readAll ();
				}
				else
				{
					Q_FOREACH (const PendingResource& pr, PendingResources_.values ())
						if (pr.URL_ == url)
							return boost::optional<QByteArray> ();

					QString location = ResourcesDir_.filePath (fileName);

					Entity e = Util::MakeEntity (url,
							location,
							LeechCraft::Internal |
								LeechCraft::DoNotNotifyUser |
								LeechCraft::DoNotSaveInHistory |
								LeechCraft::NotPersistent |
								LeechCraft::DoNotAnnounceEntity);

					int id = -1;
					QObject *pr;
					emit delegateEntity (e, &id, &pr);
					if (id == -1)
					{
						QString errorString = QString ("Could not find "
									"plugin to download %1 to %2.")
								.arg (url.toString ())
								.arg (location);
						qWarning () << Q_FUNC_INFO
								<< errorString;
						throw std::runtime_error (qPrintable (errorString));
					}

					PendingResource prdata =
					{
						url
					};

					PendingResources_ [id] = prdata;

					connect (pr,
							SIGNAL (jobFinished (int)),
							this,
							SLOT (handleResourceFinished (int)),
							Qt::UniqueConnection);
					connect (pr,
							SIGNAL (jobRemoved (int)),
							this,
							SLOT (handleResourceRemoved (int)),
							Qt::UniqueConnection);
					connect (pr,
							SIGNAL (jobError (int, IDownload::Error)),
							this,
							SLOT (handleResourceError (int, IDownload::Error)),
							Qt::UniqueConnection);
				}

				return boost::optional<QByteArray> ();
			}

			void ExternalResourceManager::ClearCaches ()
			{
				Q_FOREACH (const QString& fname, ResourcesDir_.entryList ())
					ResourcesDir_.remove (fname);
			}

			void ExternalResourceManager::ClearCachedResource (const QUrl& url)
			{
				ResourcesDir_.remove (URLToFileName (url));
			}

			void ExternalResourceManager::handleResourceFinished (int id)
			{
				if (!PendingResources_.contains (id))
					return;

				PendingResource pr = PendingResources_.take (id);

				ResourcesDir_ = GetDir ();

				emit resourceFetched (pr.URL_);
			}

			void ExternalResourceManager::handleResourceRemoved (int id)
			{
				if (!PendingResources_.contains (id))
					return;

				PendingResources_.remove (id);
			}

			void ExternalResourceManager::handleResourceError (int id, IDownload::Error error)
			{
				if (!PendingResources_.contains (id))
					return;

				qWarning () << Q_FUNC_INFO
						<< "got error"
						<< error
						<< "for PendingResource"
						<< id
						<< PendingResources_ [id].URL_;
				PendingResources_.remove (id);
			}
		}
	}
}
