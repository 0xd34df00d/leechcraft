#include <cmath>
#include <QFileInfo>
#include <QDirIterator>
#include <QtDebug>
#include <plugininterface/proxy.h>
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
    QString path = field ("RootPath").toString ();

    QDirIterator it (field ("RootPath").toString (), QDirIterator::Subdirectories);
    while (it.hasNext ())
    {
        QFileInfo info = it.fileInfo ();
        if (info.isFile () && info.isReadable ())
            TotalSize_ += info.size ();
        it.next ();
    }

	int max = std::log (static_cast<long double> (TotalSize_ / 102400)) * 80;

	int pieceSize = 32 * 1024;
	int shouldIndex = 0;
	for (; TotalSize_ / pieceSize >= max; pieceSize *= 2, ++shouldIndex);

	if (shouldIndex > PieceSize_->count () - 1)
		shouldIndex = PieceSize_->count () - 1;

	PieceSize_->setCurrentIndex (shouldIndex);

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

    NumPieces_->setText (QString::number (numPieces) +
			tr (" pieces (%1)").arg (Proxy::Instance ()->MakePrettySize (TotalSize_)));
}

