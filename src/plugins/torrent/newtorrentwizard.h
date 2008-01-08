#ifndef NEWTORRENTWIZARD_H
#define NEWTORRENTWIZARD_H
#include <QWizard>

class NewTorrentWizard : public QWizard
{
	Q_OBJECT
public:
	NewTorrentWizard (QWidget *parent = 0);
	virtual void accept ();
};

#endif

