#include <QtGui>
#include "newtorrentwizard.h"
#include "intropage.h"
#include "firststep.h"
#include "secondstep.h"

NewTorrentWizard::NewTorrentWizard (QWidget *parent)
: QWizard (parent)
{
	setWindowTitle (tr ("New torrent wizard"));

	addPage (new IntroPage);
	addPage (new FirstStep);
	addPage (new SecondStep);
}

void NewTorrentWizard::accept ()
{
	QWizard::accept ();
}

