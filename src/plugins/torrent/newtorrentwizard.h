#ifndef NEWTORRENTWIZARD_H
#define NEWTORRENTWIZARD_H
#include <QWizard>
#include "newtorrentparams.h"

class NewTorrentWizard : public QWizard
{
 Q_OBJECT
public:
 enum Page { PageIntro
  , PageFirstStep
  , PageSecondStep
  , PageThirdStep };

 NewTorrentWizard (QWidget *parent = 0);
 virtual void accept ();
 NewTorrentParams GetParams () const;
};

#endif

