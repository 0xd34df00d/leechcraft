#include "lcftp.h"
#include <QIcon>
#include <QUrl>
#include <plugininterface/util.h>
#include "core.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace LCFTP
		{
			void LCFTP::Init (ICoreProxy_ptr proxy)
			{
				Translator_.reset (LeechCraft::Util::InstallTranslator ("poshuku"));
				Core::Instance ().SetCoreProxy (proxy);
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
				return 0;
			}

			qint64 LCFTP::GetUploadSpeed () const
			{
				return 0;
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
		};
	};
};

Q_EXPORT_PLUGIN2 (leechcraft_lcftp, LeechCraft::Plugins::LCFTP::LCFTP);

