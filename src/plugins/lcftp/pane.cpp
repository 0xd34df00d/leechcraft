#include "pane.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace LCFTP
		{
			Pane::Pane (QWidget *parent)
			: QWidget (parent)
			{
				Ui_.setupUi (this);
			}
		};
	};
};

