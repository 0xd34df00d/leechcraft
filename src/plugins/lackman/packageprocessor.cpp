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

#include "packageprocessor.h"
#include <QFile>
#include <stdexcept>
#include <plugininterface/util.h>
#include "core.h"
#include "externalresourcemanager.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace LackMan
		{
			namespace
			{
				QDir GetDBDir ()
				{
					return Util::CreateIfNotExists ("lackman/filesdb/");
				}
			}

			PackageProcessor::PackageProcessor (QObject *parent)
			: QObject (parent)
			, DBDir_ (GetDBDir ())
			{
			}

			void PackageProcessor::Remove (int packageId)
			{
				QString filename = QString::number (packageId);
				if (!DBDir_.exists (filename))
				{
					qWarning () << Q_FUNC_INFO
							<< "file for package"
							<< packageId
							<< "not found";
					QString str = tr ("Could not find database file for package %1.")
							.arg (packageId);
					throw std::runtime_error (str.toUtf8 ().constData ());
				}

				QFile file (DBDir_.filePath (filename));
				if (!file.open (QIODevice::ReadOnly))
				{
					qWarning () << Q_FUNC_INFO
							<< "could not open file"
							<< file.fileName ()
							<< file.errorString ();
					QString str = tr ("Could not open database file %1: %2.")
							.arg (file.fileName ())
							.arg (file.errorString ());
					throw std::runtime_error (str.toUtf8 ().constData ());
				}

				QDir packageDir = Core::Instance ().GetPackageDir (packageId);

				QStringList files = QString::fromUtf8 (file.readAll ())
						.split ('\n', QString::SkipEmptyParts);
				Q_FOREACH (const QString& packageFilename, files)
				{
					QString fullName = packageDir.filePath (packageFilename);
					qDebug () << Q_FUNC_INFO
							<< "gonna remove"
							<< fullName;
					QFile packageFile (fullName);
					if (!packageFile.remove ())
					{
						qWarning () << Q_FUNC_INFO
								<< "could not remove file"
								<< packageFile.fileName ()
								<< packageFile.errorString ();
						QString str = tr ("Could not remove file %1: %2.")
								.arg (packageFile.fileName ())
								.arg (packageFile.errorString ());
						throw std::runtime_error (str.toUtf8 ().constData ());
					}
				}

				file.close ();
				if (!file.remove ())
				{
					qWarning () << Q_FUNC_INFO
							<< "could not delete DB file"
							<< file.fileName ()
							<< file.errorString ();
					QString str = tr ("Could not remove database file %1: %2.")
							.arg (file.fileName ())
							.arg (file.errorString ());
					throw std::runtime_error (str.toUtf8 ().constData ());
				}
			}

			void PackageProcessor::Install (int packageId)
			{
				QList<QUrl> urls = Core::Instance ().GetPackageURLs (packageId);
				if (!urls.size ())
					throw std::runtime_error (tr ("No URLs for package %1.")
							.arg (packageId).toUtf8 ().constData ());

				QUrl url = urls.at (0);

				ExternalResourceManager *erm = Core::Instance ().GetExtResourceManager ();
				connect (erm,
						SIGNAL (resourceFetched (const QUrl&)),
						this,
						SLOT (handleResourceFetched (const QUrl&)));

				if (QFile::exists (erm->GetResourcePath (url)))
				{
				}
			}
		}
	}
}
