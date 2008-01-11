#include <QFileInfo>
#include <QtDebug>
#include "thirdstep.h"
#include "secondstep.h"
#include "newtorrentwizard.h"

ThirdStep::ThirdStep (QWidget *parent)
: QWizardPage (parent)
, TotalSize_ (0)
{
	setupUi (this);
	registerField ("PieceSize", PieceSize_);
	registerField ("URLSeeds", URLSeeds_);
	registerField ("DHTEnabled", DHTEnabled_);
	registerField ("DHTNodes", DHTNodes_);
}

void ThirdStep::initializePage ()
{
	TotalSize_ = 0;
	SecondStep *spage = qobject_cast<SecondStep*> (wizard ()->page (NewTorrentWizard::PageSecondStep));
	if (!spage)
	{
		qWarning () << Q_FUNC_INFO << "failed to initialize, because seocnd page is null";
		return;
	}

	QStringList paths = spage->GetPaths ();

	for (int i = 0; i < paths.size (); ++i)
		TotalSize_ += QFileInfo (paths.at (i)).size ();

	on_PieceSize__currentIndexChanged ();
}

void ThirdStep::on_PieceSize__currentIndexChanged ()
{
	int mul = 32 * 1024;
	int index = PieceSize_->currentIndex ();
	while (index--)
		mul *= 2;

	int numPieces = TotalSize_ / mul;
	if (TotalSize_ % mul)
		++numPieces;

	NumPieces_->setText (QString::number (numPieces) + tr (" pieces"));
}

