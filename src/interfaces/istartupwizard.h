#ifndef INTERFACES_ISTARTUPWIZARD_H
#define INTERFACES_ISTARTUPWIZARD_H
#include <QList>

class QWizardPage;

class IStartupWizard
{
public:
	virtual QList<QWizardPage*> GetWizardPages () const = 0;
};

Q_DECLARE_INTERFACE (IStartupWizard, "org.Deviant.LeechCraft.IStartupWizard/1.0");

#endif

