#include "cstp.h"
#include <boost/bind.hpp>
#include <boost/logic/tribool.hpp>
#include <QMenu>
#include <QTranslator>
#include <QLocale>
#include <QFileInfo>
#include <QTabWidget>
#include <QToolBar>
#include <QMessageBox>
#include <QModelIndex>
#include <QDir>
#include <QUrl>
#include <QTextCodec>
#include <QTranslator>
#include <plugininterface/util.h>
#include "core.h"
#include "xmlsettingsmanager.h"
#include "mainviewdelegate.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace CSTP
		{
			CSTP::~CSTP ()
			{
			}
			
			void CSTP::Init (ICoreProxy_ptr coreProxy)
			{
				Core::Instance ().SetCoreProxy (coreProxy);
				Translator_.reset (LeechCraft::Util::InstallTranslator ("cstp"));
			
				XmlSettingsDialog_.reset (new LeechCraft::Util::XmlSettingsDialog ());
				XmlSettingsDialog_->RegisterObject (&XmlSettingsManager::Instance (),
						"cstpsettings.xml");
			
				SetupToolbar ();
			
				Core::Instance ().SetToolbar (Toolbar_.get ());
			
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
						SIGNAL (error (const QString&)),
						this,
						SIGNAL (log (const QString&)));
			}
			
			void CSTP::Release ()
			{
				Core::Instance ().Release ();
				XmlSettingsManager::Instance ().Release ();
				XmlSettingsDialog_.reset ();
				Toolbar_.reset ();
				Translator_.reset ();
			}
			
			QString CSTP::GetName () const
			{
				return "CSTP";
			}
			
			QString CSTP::GetInfo () const
			{
				return "Common Stream Transfer Protocols";
			}
			
			QStringList CSTP::Provides () const
			{
				return QStringList ("http") << "https" << "remoteable" << "resume";
			}
			
			QStringList CSTP::Needs () const
			{
				return QStringList ();
			}
			
			QStringList CSTP::Uses () const
			{
				return QStringList ();
			}
			
			void CSTP::SetProvider (QObject*, const QString&)
			{
			}
			
			QIcon CSTP::GetIcon () const
			{
				return QIcon ();
			}
			
			qint64 CSTP::GetDownloadSpeed () const
			{
				return Core::Instance ().GetTotalDownloadSpeed ();
			}
			
			qint64 CSTP::GetUploadSpeed () const
			{
				return 0;
			}
			
			void CSTP::StartAll ()
			{
				Core::Instance ().startAllTriggered ();
			}
			
			void CSTP::StopAll ()
			{
				Core::Instance ().stopAllTriggered ();
			}
			
			bool CSTP::CouldDownload (const LeechCraft::DownloadEntity& e) const
			{
				return Core::Instance ().CouldDownload (e);
			}
			
			int CSTP::AddJob (LeechCraft::DownloadEntity e)
			{
				return Core::Instance ().AddTask (e);
			}
			
			QAbstractItemModel* CSTP::GetRepresentation () const
			{
				return Core::Instance ().GetRepresentationModel ();
			}
			
			void CSTP::ItemSelected (const QModelIndex& index)
			{
				Core::Instance ().ItemSelected (index);
			}
			
			boost::shared_ptr<LeechCraft::Util::XmlSettingsDialog> CSTP::GetSettingsDialog () const
			{
				return XmlSettingsDialog_;
			}
			
			template<typename T>
			void CSTP::ApplyCore2Selection (void (Core::*temp) (const QModelIndex&), T view)
			{
				QModelIndexList indexes = view->selectionModel ()->
					selectedRows ();
				std::for_each (indexes.begin (), indexes.end (),
						boost::bind (temp,
							&Core::Instance (),
							_1));
			}
			
			void CSTP::SetupToolbar ()
			{
				Toolbar_.reset (new QToolBar);
				Toolbar_->setMovable (false);
				Toolbar_->setFloatable (false);
			
				QAction *remove = Toolbar_->addAction (tr ("Remove"));
				connect (remove,
						SIGNAL (triggered ()),
						&Core::Instance (),
						SLOT (removeTriggered ()));
				remove->setProperty ("ActionIcon", "cstp_remove");
			
				QAction *removeAll = Toolbar_->addAction (tr ("Remove all"));
				connect (removeAll,
						SIGNAL (triggered ()),
						&Core::Instance (),
						SLOT (removeAllTriggered ()));
				removeAll->setProperty ("ActionIcon", "cstp_removeall");
			
				Toolbar_->addSeparator ();
			
				QAction *start = Toolbar_->addAction (tr ("Start"));
				connect (start,
						SIGNAL (triggered ()),
						&Core::Instance (),
						SLOT (startTriggered ()));
				start->setProperty ("ActionIcon", "cstp_start");
			
				QAction *stop = Toolbar_->addAction (tr ("Stop"));
				connect (stop,
						SIGNAL (triggered ()),
						&Core::Instance (),
						SLOT (stopTriggered ()));
				stop->setProperty ("ActionIcon", "cstp_stop");
			
				QAction *startAll = Toolbar_->addAction (tr ("Start all"));
				connect (startAll,
						SIGNAL (triggered ()),
						&Core::Instance (),
						SLOT (startAllTriggered ()));
				startAll->setProperty ("ActionIcon", "cstp_startall");
			
				QAction *stopAll = Toolbar_->addAction (tr ("Stop all"));
				connect (stopAll,
						SIGNAL (triggered ()),
						&Core::Instance (),
						SLOT (stopAllTriggered ()));
				stopAll->setProperty ("ActionIcon", "cstp_stopall");
			}
			
			void CSTP::handleFileExists (boost::logic::tribool *remove)
			{
				QMessageBox::StandardButton userReply =
					QMessageBox::warning (0,
						tr ("File exists"),
						tr ("File %1 already exists, continue download?"),
						QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
				if (userReply == QMessageBox::Yes)
					*remove = false;
				else if (userReply == QMessageBox::No)
					*remove = true;
				else
					*remove = boost::logic::indeterminate;
			}
			
			Q_EXPORT_PLUGIN2 (leechcraft_cstp, CSTP);
		};
	};
};

