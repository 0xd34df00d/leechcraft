#include "seekthru.h"
#include <QMessageBox>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include "core.h"
#include "xmlsettingsmanager.h"
#include "searcherslist.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace SeekThru
		{
			using namespace LeechCraft::Util;
			
			void SeekThru::Init (ICoreProxy_ptr)
			{
				connect (&Core::Instance (),
						SIGNAL (delegateEntity (const LeechCraft::DownloadEntity&,
								int*, QObject**)),
						this,
						SIGNAL (delegateEntity (const LeechCraft::DownloadEntity&,
								int*, QObject**)));
				connect (&Core::Instance (),
						SIGNAL (gotEntity (const LeechCraft::DownloadEntity&)),
						this,
						SIGNAL (gotEntity (const LeechCraft::DownloadEntity&)));
				connect (&Core::Instance (),
						SIGNAL (error (const QString&)),
						this,
						SLOT (handleError (const QString&)));
				connect (&Core::Instance (),
						SIGNAL (warning (const QString&)),
						this,
						SLOT (handleWarning (const QString&)));
			
				XmlSettingsDialog_.reset (new XmlSettingsDialog ());
				XmlSettingsDialog_->RegisterObject (&XmlSettingsManager::Instance (),
						"seekthrusettings.xml");
				XmlSettingsDialog_->SetCustomWidget ("SearchersList", new SearchersList);
			}
			
			void SeekThru::Release ()
			{
				XmlSettingsDialog_.reset ();
			}
			
			QString SeekThru::GetName () const
			{
				return "SeekThru";
			}
			
			QString SeekThru::GetInfo () const
			{
				return tr ("Search via OpenSearch-aware search providers.");
			}
			
			QIcon SeekThru::GetIcon () const
			{
				return QIcon (":/resources/images/seekthru.png");
			}
			
			QStringList SeekThru::Provides () const
			{
				return QStringList ("search");
			}
			
			QStringList SeekThru::Needs () const
			{
				return QStringList ("http");
			}
			
			QStringList SeekThru::Uses () const
			{
				return QStringList ("webbrowser");
			}
			
			void SeekThru::SetProvider (QObject *object, const QString& feature)
			{
				Core::Instance ().SetProvider (object, feature);
			}
			
			QStringList SeekThru::GetCategories () const
			{
				return Core::Instance ().GetCategories ();
			}
			
			IFindProxy_ptr SeekThru::GetProxy (const LeechCraft::Request& r)
			{
				return Core::Instance ().GetProxy (r);
			}
			
			boost::shared_ptr<LeechCraft::Util::XmlSettingsDialog> SeekThru::GetSettingsDialog () const
			{
				return XmlSettingsDialog_;
			}
			
			bool SeekThru::CouldHandle (const LeechCraft::DownloadEntity& e) const
			{
				return Core::Instance ().CouldHandle (e);
			}
			
			void SeekThru::Handle (LeechCraft::DownloadEntity e)
			{
				Core::Instance ().Add (e.Location_);
			}
			
			void SeekThru::handleError (const QString& error)
			{
				QMessageBox::critical (0,
						tr ("LeechCraft"),
						error);
			}
			
			void SeekThru::handleWarning (const QString& error)
			{
				QMessageBox::warning (0,
						tr ("LeechCraft"),
						error);
			}
			
			Q_EXPORT_PLUGIN2 (leechcraft_seekthru, SeekThru);
			
		};
	};
};

