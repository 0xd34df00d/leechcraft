#include "lcftp.h"
#include <QIcon>
#include <QUrl>
#include <plugininterface/util.h>
#include "core.h"
#include "xmlsettingsmanager.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace LCFTP
		{
			void LCFTP::Init (ICoreProxy_ptr proxy)
			{
				Translator_.reset (LeechCraft::Util::InstallTranslator ("poshuku"));

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
				return QIcon ();
			}

			QAbstractItemModel* LCFTP::GetRepresentation () const
			{
				return Core::Instance ().GetModel ();
			}

			void LCFTP::ItemSelected (const QModelIndex&)
			{
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

			bool LCFTP::CouldHandle (const DownloadEntity& e) const
			{
				return Core::Instance ().IsOK (e);
			}

			void LCFTP::Handle (DownloadEntity e)
			{
				Core::Instance ().Add (e);
			}

			boost::shared_ptr<LeechCraft::Util::XmlSettingsDialog> LCFTP::GetSettingsDialog () const
			{
				return XmlSettingsDialog_;
			}
		};
	};
};

Q_EXPORT_PLUGIN2 (leechcraft_lcftp, LeechCraft::Plugins::LCFTP::LCFTP);

