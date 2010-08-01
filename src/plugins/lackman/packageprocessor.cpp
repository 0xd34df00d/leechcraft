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
#include <QDirIterator>
#include <QFileInfo>
#include <stdexcept>
#include <plugininterface/util.h>
#include "core.h"
#include "externalresourcemanager.h"
#include "storage.h"

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

				QByteArray fileData = file.readAll ();
				QStringList files = QString::fromUtf8 (fileData)
						.split ('\n', QString::SkipEmptyParts);
				files.sort ();
				std::reverse (files.begin (), files.end ());
				Q_FOREACH (const QString& packageFilename, files)
				{
					QString fullName = packageDir.filePath (packageFilename);
#ifndef QT_NO_DEBUG
					qDebug () << Q_FUNC_INFO
							<< "gonna remove"
							<< fullName;
#endif
					QFileInfo fi (fullName);

					if (fi.isFile ())
					{
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
					else if (fi.isDir ())
					{
						if (!QDir (fi.path ()).rmdir (fi.fileName ()))
						{
							qWarning () << Q_FUNC_INFO
									<< "could not remove directory"
									<< fullName
									<< "→ parent:"
									<< fi.path ()
									<< "; child node:"
									<< fi.fileName ();
							QString str = tr ("Could not remove directory %1.")
									.arg (fi.fileName ());
							throw std::runtime_error (str.toUtf8 ().constData ());
						}
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
				QUrl url = GetURLFor (packageId);

				ExternalResourceManager *erm = PrepareResourceManager ();

				if (QFile::exists (erm->GetResourcePath (url)))
					HandleFile (packageId, url, MInstall);
				else
				{
					URL2Id_ [url] = packageId;
					URL2Mode_ [url] = MInstall;
					erm->GetResourceData (url);
				}
			}

			void PackageProcessor::Update (int toPackageId)
			{
				QUrl url = GetURLFor (toPackageId);

				ExternalResourceManager *erm = PrepareResourceManager ();

				if (QFile::exists (erm->GetResourcePath (url)))
					HandleFile (toPackageId, url, MUpdate);
				else
				{
					URL2Id_ [url] = toPackageId;
					URL2Mode_ [url] = MUpdate;
					erm->GetResourceData (url);
				}
			}

			void PackageProcessor::handleResourceFetched (const QUrl& url)
			{
				if (URL2Id_.contains (url))
					HandleFile (URL2Id_.take (url), url, URL2Mode_.take (url));
			}

			void PackageProcessor::handlePackageUnarchFinished (int ret, QProcess::ExitStatus status)
			{
				sender ()->deleteLater ();

				QProcess *unarch = qobject_cast<QProcess*> (sender ());
				int packageId = unarch->property ("PackageID").toInt ();
				QString stagingDir = unarch->property ("StagingDirectory").toString ();
				Mode mode = static_cast<Mode> (unarch->property ("Mode").toInt ());

				if (ret)
				{
					QString stderr = QString::fromUtf8 (unarch->readAllStandardError ());
					qWarning () << Q_FUNC_INFO
							<< "unpacker exited with"
							<< ret
							<< stderr
							<< "for"
							<< packageId
							<< unarch->property ("Path").toString ();

					QString errorString = tr ("Unable to unpack package archive, unpacker exited with %1: %2.")
							.arg (ret)
							.arg (stderr);
					emit packageInstallError (packageId, errorString);

					CleanupDir (stagingDir);

					return;
				}

				int oldId = -1;
				if (mode == MUpdate)
				{
					oldId = Core::Instance ().GetStorage ()->FindInstalledPackage (packageId);
					if (!CleanupBeforeUpdate (oldId, packageId))
					{
						qWarning () << Q_FUNC_INFO
								<< "unable to cleanup";
						return;
					}
				}

				QDir packageDir;
				try
				{
					packageDir = Core::Instance ().GetPackageDir (packageId);
				}
				catch (const std::exception& e)
				{
					qWarning () << Q_FUNC_INFO
							<< "while trying to get dir for package"
							<< packageId
							<< "got we exception"
							<< e.what ();
					QString errorString = tr ("Unable to get directory for the package: %1.")
							.arg (QString::fromUtf8 (e.what ()));
					emit packageInstallError (packageId, errorString);
					CleanupDir (stagingDir);
					return;
				}

				QDirIterator dirIt (stagingDir,
						QDir::NoDotAndDotDot |
							QDir::Readable |
							QDir::NoSymLinks |
							QDir::Dirs |
							QDir::Files,
						QDirIterator::Subdirectories);
				while (dirIt.hasNext ())
				{
					dirIt.next ();
					QFileInfo fi = dirIt.fileInfo ();

					if (fi.isDir () ||
							fi.isFile ())
						if (!HandleEntry (packageId, fi, stagingDir, packageDir))
						{
							try
							{
								Remove (packageId);
							}
							catch (const std::exception& e)
							{
								qWarning () << Q_FUNC_INFO
										<< "while removing partially installed package"
										<< packageId
										<< "got:"
										<< e.what ();
							}

							QString errorString = tr ("Unable to copy "
									"files from staging area to "
									"destination directory.");
							emit packageInstallError (packageId, errorString);
							CleanupDir (stagingDir);
							return;
						}
				}

				switch (mode)
				{
				case MInstall:
					emit packageInstalled (packageId);
					break;
				case MUpdate:
					emit packageUpdated (oldId, packageId);
					break;
				}
			}

			void PackageProcessor::handleUnarchError (QProcess::ProcessError error)
			{
				sender ()->deleteLater ();

				QByteArray stderr = qobject_cast<QProcess*> (sender ())->readAllStandardError ();
				qWarning () << Q_FUNC_INFO
						<< "unable to unpack for"
						<< sender ()->property ("PackageID").toInt ()
						<< sender ()->property ("Path").toString ()
						<< "with"
						<< error
						<< stderr;

				QString errorString = tr ("Unable to unpack package archive, unpacker died with %1: %2.")
						.arg (error)
						.arg (QString::fromUtf8 (stderr));
				emit packageInstallError (sender ()->property ("PackageID").toInt (),
						errorString);

				CleanupDir (sender ()->property ("StagingDirectory").toString ());
			}

			void PackageProcessor::HandleFile (int packageId,
					const QUrl& url, PackageProcessor::Mode mode)
			{
				QString path = Core::Instance ()
						.GetExtResourceManager ()->GetResourcePath (url);

				QProcess *unarch = new QProcess (this);
				connect (unarch,
						SIGNAL (finished (int, QProcess::ExitStatus)),
						this,
						SLOT (handlePackageUnarchFinished (int, QProcess::ExitStatus)));
				connect (unarch,
						SIGNAL (error (QProcess::ProcessError)),
						this,
						SLOT (handleUnarchError (QProcess::ProcessError)));

				QString dirname = Util::GetTemporaryName ("lackman_stagingarea");
				QStringList args;
				args << "xvzf";
				args << path;
				args << "-C";
				args << dirname;
				unarch->setProperty ("PackageID", packageId);
				unarch->setProperty ("StagingDirectory", dirname);
				unarch->setProperty ("Path", path);
				unarch->setProperty ("Mode", mode);

				QFileInfo sdInfo (dirname);
				QDir stagingParentDir (sdInfo.path ());
				if (!stagingParentDir.exists (sdInfo.fileName ()) &&
						!stagingParentDir.mkdir (sdInfo.fileName ()))
				{
					qWarning () << Q_FUNC_INFO
							<< "unable to create staging directory"
							<< sdInfo.fileName ()
							<< "in"
							<< sdInfo.path ();

					QString errorString = tr ("Unable to create staging directory %1.")
							.arg (sdInfo.fileName ());
					emit packageInstallError (packageId, errorString);

					return;
				}

				unarch->start ("tar", args);
			}

			QUrl PackageProcessor::GetURLFor (int packageId) const
			{
				QList<QUrl> urls = Core::Instance ().GetPackageURLs (packageId);
				if (!urls.size ())
					throw std::runtime_error (tr ("No URLs for package %1.")
							.arg (packageId).toUtf8 ().constData ());

				QUrl url = urls.at (0);
				qDebug () << Q_FUNC_INFO
						<< "would fetch"
						<< packageId
						<< "from"
						<< url;
				return url;
			}

			ExternalResourceManager* PackageProcessor::PrepareResourceManager ()
			{
				ExternalResourceManager *erm = Core::Instance ().GetExtResourceManager ();
				connect (erm,
						SIGNAL (resourceFetched (const QUrl&)),
						this,
						SLOT (handleResourceFetched (const QUrl&)),
						Qt::UniqueConnection);
				return erm;
			}

			bool PackageProcessor::HandleEntry (int packageId, const QFileInfo& fi,
					const QString& stagingDir, QDir& packageDir)
			{
				QFile dbFile (DBDir_.filePath (QString::number (packageId)));
				if (!dbFile.open (QIODevice::WriteOnly | QIODevice::Append))
				{
					qWarning () << Q_FUNC_INFO
							<< "could not open DB file"
							<< dbFile.fileName ()
							<< "for write:"
							<< dbFile.errorString ();
					return false;
				}

				QString sourceName = fi.absoluteFilePath ();
				sourceName = sourceName.mid (stagingDir.length ());
				if (sourceName.at (0) == '/')
					sourceName = sourceName.mid (1);

				if (fi.isFile ())
				{
					QString destName = packageDir.filePath (sourceName);
#ifndef QT_NO_DEBUG
					qDebug () << Q_FUNC_INFO
							<< "gotta copy"
							<< fi.absoluteFilePath ()
							<< "to"
							<< destName;
#endif

					QFile file (fi.absoluteFilePath ());
					if (!file.copy (destName))
					{
						qWarning () << Q_FUNC_INFO
								<< "could not copy"
								<< fi.absoluteFilePath ()
								<< "to"
								<< destName
								<< "because of"
								<< file.errorString ();

						QString errorString = tr ("Could not copy file %1 because of %2.")
								.arg (sourceName)
								.arg (file.errorString ());
						emit packageInstallError (packageId, errorString);
						return false;
					}
				}
				else if (fi.isDir ())
				{
#ifndef QT_NO_DEBUG
					qDebug () << Q_FUNC_INFO
							<< "gotta create"
							<< sourceName
							<< "for"
							<< fi.absoluteFilePath ();
#endif

					if (!packageDir.mkpath (sourceName))
					{
						qWarning () << Q_FUNC_INFO
								<< "unable to mkdir"
								<< sourceName
								<< "in"
								<< packageDir.path ();

						QString errorString = tr ("Unable to create directory %1.")
								.arg (sourceName);
						emit packageInstallError (packageId, errorString);
						return false;
					}
				}

				dbFile.write (sourceName.toUtf8 ());
				dbFile.write ("\n");

				return true;
			}

			bool PackageProcessor::CleanupBeforeUpdate (int oldId, int newId)
			{
				try
				{
					Remove (oldId);
				}
				catch (const std::exception& e)
				{
					qWarning () << Q_FUNC_INFO
							<< "while removing package"
							<< oldId
							<< "for update to"
							<< newId
							<< "got exception:"
							<< e.what ();
					QString str = tr ("Unable to remove package %1 "
								"when updating to package %2")
							.arg (oldId)
							.arg (newId);
					emit packageInstallError (newId, str);
					return false;
				}
				return true;
			}

			void PackageProcessor::CleanupDir (const QString& directory)
			{
				// TODO
			}
		}
	}
}
