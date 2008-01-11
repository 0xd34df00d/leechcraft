#include <QtGui>
#include "newtorrentwizard.h"
#include "intropage.h"
#include "firststep.h"
#include "secondstep.h"
#include "thirdstep.h"

NewTorrentWizard::NewTorrentWizard (QWidget *parent)
: QWizard (parent)
{
	setWindowTitle (tr ("New torrent wizard"));

	setPage (PageIntro, new IntroPage);
	setPage (PageFirstStep, new FirstStep);
	setPage (PageSecondStep, new SecondStep);
	setPage (PageThirdStep, new ThirdStep);
}

void NewTorrentWizard::accept ()
{
	QWizard::accept ();
}

NewTorrentParams NewTorrentWizard::GetParams () const
{
	NewTorrentParams result;

	result.OutputDirectory_ = field ("OutputDirectory").toString ();
	result.TorrentName_ = field ("TorrentName").toString ();
	result.AnnounceURL_ = field ("AnnounceURL").toString ();
	result.Date_ = field ("Date").toDate ();
	result.Comment_ = field ("Comment").toString ();
	result.Paths_ = qobject_cast<SecondStep*> (page (PageSecondStep))->GetPaths ();
	result.PieceSize_ = field ("PieceSize").toInt ();
	result.URLSeeds_ = field ("URLSeeds").toString ().split(QRegExp("\\s+"));
	result.DHTEnabled_ = field ("DHTEnabled").toBool ();
	result.DHTNodes_ = field ("DHTNodes").toString ().split(QRegExp("\\s+"));

	return result;
}

