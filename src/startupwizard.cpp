#include "startupwizard.h"
#include "interfaces/istartupwizard.h"
#include "core.h"

namespace LeechCraft
{
	StartupWizard::StartupWizard (QWidget *parent)
	: QWizard (parent)
	{
		setWindowTitle (tr ("Startup wizard"));
		setAttribute (Qt::WA_DeleteOnClose);

		QList<IStartupWizard*> wizards = Core::Instance ()
			.GetPluginManager ()->GetAllCastableTo<IStartupWizard*> ();

		QList<QWizardPage*> pages;
		Q_FOREACH (IStartupWizard *wizard, wizards)
			pages += wizard->GetWizardPages ();

		if (!pages.size ())
		{
			deleteLater ();
			return;
		}

		Q_FOREACH (QWizardPage *page, pages)
				addPage (page);

		show ();
	}
};
