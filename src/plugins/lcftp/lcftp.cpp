/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
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

#include "lcftp.h"
#include <QIcon>
#include <QUrl>
#include <interfaces/entitytesthandleresult.h>
#include <util/util.h>
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
				TabWidget::SetParentMultiTabs (this);

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
						SIGNAL (gotEntity (const LeechCraft::Entity&)),
						this,
						SIGNAL (gotEntity (const LeechCraft::Entity&)));

				TabManager_ = Core::Instance ().GetTabManager ();
				connect (TabManager_,
						SIGNAL (addNewTab (const QString&, QWidget*)),
						this,
						SIGNAL (addNewTab (const QString&, QWidget*)));
				connect (TabManager_,
						SIGNAL (removeTab (QWidget*)),
						this,
						SIGNAL (removeTab (QWidget*)));
				connect (TabManager_,
						SIGNAL (changeTabName (QWidget*, const QString&)),
						this,
						SIGNAL (changeTabName (QWidget*, const QString&)));
				connect (TabManager_,
						SIGNAL (changeTabIcon (QWidget*, const QIcon&)),
						this,
						SIGNAL (changeTabIcon (QWidget*, const QIcon&)));
				connect (TabManager_,
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

			QByteArray LCFTP::GetUniqueID () const
			{
				return "org.LeechCraft.LCFTP";
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
			
			TabClasses_t LCFTP::GetTabClasses () const
			{
				TabClasses_t result;
				result << Core::Instance ().GetTabClass ();
				return result;
			}
			
			void LCFTP::TabOpenRequested (const QByteArray& tabClass)
			{
				if (tabClass == "LCFTP")
					TabManager_->AddTab (QUrl (), QString ());
				else
					qWarning () << Q_FUNC_INFO
							<< "unknown tab class"
							<< tabClass;
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

			EntityTestHandleResult LCFTP::CouldDownload (const Entity& e) const
			{
				return Core::Instance ().IsOK (e) ?
						EntityTestHandleResult (EntityTestHandleResult::PIdeal) :
						EntityTestHandleResult ();
			}

			int LCFTP::AddJob (Entity e)
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

			EntityTestHandleResult LCFTP::CouldHandle (const Entity& e) const
			{
				return Core::Instance ().IsOK (e) ?
						EntityTestHandleResult (EntityTestHandleResult::PIdeal) :
						EntityTestHandleResult ();
			}

			void LCFTP::Handle (Entity e)
			{
				Core::Instance ().Handle (e);
			}

			std::shared_ptr<LeechCraft::Util::XmlSettingsDialog> LCFTP::GetSettingsDialog () const
			{
				return XmlSettingsDialog_;
			}
		};
	};
};

LC_EXPORT_PLUGIN (leechcraft_lcftp, LeechCraft::Plugins::LCFTP::LCFTP);

