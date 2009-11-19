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

#include "lcftp.h"
#include <QIcon>
#include <QUrl>
#include <plugininterface/util.h>
#include "core.h"
#include "tabmanager.h"
#include "xmlsettingsmanager.h"

#ifdef AddJob
#undef AddJob
#endif

namespace LeechCraft
{
	namespace Plugins
	{
		namespace LCFTP
		{
			void LCFTP::Init (ICoreProxy_ptr proxy)
			{
				Translator_.reset (LeechCraft::Util::InstallTranslator ("lcftp"));

				XmlSettingsDialog_.reset (new LeechCraft::Util::XmlSettingsDialog ());
				XmlSettingsDialog_->RegisterObject (&XmlSettingsManager::Instance (),
						"lcftpsettings.xml");

				Core::Instance ().SetCoreProxy (proxy);

				connect (&Core::Instance (),
						SIGNAL (taskFinished (int)),
						this,
						SIGNAL (jobFinished (int)));
				connect (&Core::Instance (),
						SIGNAL (taskRemoved (int)),
						this,
						SIGNAL (jobRemoved (int)));
				connect (&Core::Instance (),
						SIGNAL (taskError (int, IDownload::Error)),
						this,
						SIGNAL (jobError (int, IDownload::Error)));
				connect (&Core::Instance (),
						SIGNAL (gotEntity (const LeechCraft::DownloadEntity&)),
						this,
						SIGNAL (gotEntity (const LeechCraft::DownloadEntity&)));
				connect (&Core::Instance (),
						SIGNAL (downloadFinished (const QString&)),
						this,
						SIGNAL (downloadFinished (const QString&)));
				connect (&Core::Instance (),
						SIGNAL (log (const QString&)),
						this,
						SIGNAL (log (const QString&)));

				TabManager *manager = Core::Instance ().GetTabManager ();
				connect (manager,
						SIGNAL (addNewTab (const QString&, QWidget*)),
						this,
						SIGNAL (addNewTab (const QString&, QWidget*)));
				connect (manager,
						SIGNAL (removeTab (QWidget*)),
						this,
						SIGNAL (removeTab (QWidget*)));
				connect (manager,
						SIGNAL (changeTabName (QWidget*, const QString&)),
						this,
						SIGNAL (changeTabName (QWidget*, const QString&)));
				connect (manager,
						SIGNAL (changeTabIcon (QWidget*, const QIcon&)),
						this,
						SIGNAL (changeTabIcon (QWidget*, const QIcon&)));
				connect (manager,
						SIGNAL (statusBarChanged (QWidget*, const QString&)),
						this,
						SIGNAL (statusBarChanged (QWidget*, const QString&)));
			}

			void LCFTP::SecondInit ()
			{
			}

			void LCFTP::Release ()
			{
				Core::Instance ().Release ();
				Translator_.reset ();
			}

			QString LCFTP::GetName () const
			{
				return "LCFTP";
			}

			QString LCFTP::GetInfo () const
			{
				return tr ("A simple FTP client");
			}

			QStringList LCFTP::Provides () const
			{
				return Core::Instance ().Provides ();
			}

			QStringList LCFTP::Needs () const
			{
				return QStringList ();
			}

			QStringList LCFTP::Uses () const
			{
				return QStringList ();
			}

			void LCFTP::SetProvider (QObject*, const QString&)
			{
			}

			QIcon LCFTP::GetIcon () const
			{
				return QIcon (":/resources/images/lcftp.svg");
			}

			QAbstractItemModel* LCFTP::GetRepresentation () const
			{
				return Core::Instance ().GetModel ();
			}

			qint64 LCFTP::GetDownloadSpeed () const
			{
				return Core::Instance ().GetDownloadSpeed ();
			}

			qint64 LCFTP::GetUploadSpeed () const
			{
				return Core::Instance ().GetUploadSpeed ();
			}

			void LCFTP::StartAll ()
			{
			}

			void LCFTP::StopAll ()
			{
			}

			bool LCFTP::CouldDownload (const DownloadEntity& e) const
			{
				return Core::Instance ().IsOK (e);
			}

			int LCFTP::AddJob (DownloadEntity e)
			{
				return Core::Instance ().Add (e);
			}

			void LCFTP::KillTask (int id)
			{
				Q_UNUSED (id);
				// TODO implement
				// remember that id is the id of the task obtained by
				// the call to GetID() and returned from AddJob.
			}

			bool LCFTP::CouldHandle (const DownloadEntity& e) const
			{
				return Core::Instance ().IsOK (e);
			}

			void LCFTP::Handle (DownloadEntity e)
			{
				Core::Instance ().Handle (e);
			}

			boost::shared_ptr<LeechCraft::Util::XmlSettingsDialog> LCFTP::GetSettingsDialog () const
			{
				return XmlSettingsDialog_;
			}
		};
	};
};

Q_EXPORT_PLUGIN2 (leechcraft_lcftp, LeechCraft::Plugins::LCFTP::LCFTP);

