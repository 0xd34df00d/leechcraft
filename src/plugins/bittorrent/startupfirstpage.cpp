#include "startupfirstpage.h"
#include "xmlsettingsmanager.h"
#include "core.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace BitTorrent
		{
			StartupFirstPage::StartupFirstPage (QWidget *parent)
			: QWizardPage (parent)
			{
				Ui_.setupUi (this);

				setTitle (tr ("BitTorrent"));
				setSubTitle (tr ("Set basic options"));
			}

			void StartupFirstPage::initializePage ()
			{
				connect (wizard (),
						SIGNAL (accepted ()),
						this,
						SLOT (handleAccepted ()));
			}

			void StartupFirstPage::handleAccepted ()
			{
				QList<QVariant> ports;
				ports << Ui_.LowerPort_->value ()
					<< Ui_.UpperPort_->value ();
				XmlSettingsManager::Instance ()->setProperty ("TCPPortRange", ports);

				XmlSettingsManager::Instance ()->setProperty ("MaxUploads",
						Ui_.UploadConnections_->value ());
				XmlSettingsManager::Instance ()->setProperty ("MaxConnections",
						Ui_.TotalConnections_->value ());

				int sset = Ui_.SettingsSet_->currentIndex ();
				Core::Instance ()->SetPreset (static_cast<Core::SettingsPreset> (sset));
			}
		};
	};
};

