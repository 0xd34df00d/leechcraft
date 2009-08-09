#ifndef STARTUPWIZARD_H
#define STARTUPWIZARD_H
#include <QWizard>

namespace LeechCraft
{
	class StartupWizard : public QWizard
	{
		Q_OBJECT
	public:
		StartupWizard (QWidget* = 0);
	};
};

#endif

