#include "addwebseeddialog.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace BitTorrent
		{
			AddWebSeedDialog::AddWebSeedDialog (QWidget *parent)
			: QDialog (parent)
			{
				Ui_.setupUi (this);
			}

			QString AddWebSeedDialog::GetURL () const
			{
				return Ui_.URL_->text ();
			}

			bool AddWebSeedDialog::GetType () const
			{
				return Ui_.BEP19_->isChecked ();
			}
		};
	};
};

