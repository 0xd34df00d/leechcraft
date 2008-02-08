#include <QFileInfo>
#include <QDirIterator>
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
   QString path = field ("RootPath").toString ();

   QDirIterator it (field ("RootPath").toString (), QDirIterator::Subdirectories);
   while (it.hasNext ())
   {
      QFileInfo info = it.fileInfo ();
      if (info.isFile () && info.isReadable ())
         TotalSize_ += info.size ();
      it.next ();
   }

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

