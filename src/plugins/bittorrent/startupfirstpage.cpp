#include "startupfirstpage.h"
#include "xmlsettingsmanager.h"

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
			}
		};
	};
};
