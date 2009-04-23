#ifndef PLUGININTERFACE_BACKENDSELECTOR_H
#define PLUGININTERFACE_BACKENDSELECTOR_H
#include <QWidget>
#include "config.h"
#include "../xmlsettingsdialog/basesettingsmanager.h"

namespace Ui
{
	class BackendSelector;
};

namespace LeechCraft
{
	namespace Util
	{
		class LEECHCRAFT_API BackendSelector : public QWidget
		{
			Q_OBJECT

			Ui::BackendSelector *Ui_;
			BaseSettingsManager *Manager_;
		public:
			BackendSelector (BaseSettingsManager*, QWidget* = 0);
		private:
			void FillUI ();
		public slots:
			void accept ();
			void reject ();
		};
	};
};

#endif

