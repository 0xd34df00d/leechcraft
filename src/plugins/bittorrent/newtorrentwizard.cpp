#include <QtGui/QtGui>
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
    setPage (PageSecondStep, new ThirdStep);
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
    result.Path_ = field ("RootPath").toString ();
    result.URLSeeds_ = field ("URLSeeds").toString ().split(QRegExp("\\s+"));
    result.DHTEnabled_ = field ("DHTEnabled").toBool ();
    result.DHTNodes_ = field ("DHTNodes").toString ().split(QRegExp("\\s+"));
    result.PieceSize_ = 32 * 1024;
    int index = field ("PieceSize").toInt ();
    while (index--)
        result.PieceSize_ *= 2;

    if (result.Path_.endsWith ('/'))
        result.Path_.remove (result.Path_.size () - 1, 1);

    return result;
}

