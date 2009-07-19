#include "lcftp.h"
#include <QIcon>
#include <plugininterface/util.h>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace LCFTP
		{
			void LCFTP::Init (ICoreProxy_ptr)
			{
				Translator_.reset (LeechCraft::Util::InstallTranslator ("poshuku"));
			}

			void LCFTP::Release ()
			{
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
				return QStringList ("ftp");
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
				return false;
			}

			int LCFTP::AddJob (DownloadEntity e)
			{
			}

			bool LCFTP::CouldHandle (const DownloadEntity& e) const
			{
				return false;
			}

			void LCFTP::Handle (DownloadEntity e)
			{
			}
		};
	};
};

Q_EXPORT_PLUGIN2 (leechcraft_lcftp, LeechCraft::Plugins::LCFTP::LCFTP);

