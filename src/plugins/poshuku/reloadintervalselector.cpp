#include "reloadintervalselector.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Poshuku
		{
			ReloadIntervalSelector::ReloadIntervalSelector (QWidget *parent)
			: QDialog (parent)
			{
				Ui_.setupUi (this);
			}

			QTime ReloadIntervalSelector::GetInterval () const
			{
				return Ui_.Interval_->time ();
			}
		};
	};
};

