#include <QLabel>
#include <QVBoxLayout>
#include "intropage.h"

IntroPage::IntroPage (QWidget *parent)
: QWizardPage (parent)
{
    setTitle (tr ("Introduction"));
    Label_ = new QLabel (tr ("This wizard will generate a torrent file. "
                             "You simply need so specify the torrent "
                             "name, files to include and optionally few "
                             "other options to produce your torrent file."));
    Label_->setWordWrap (true);
    QVBoxLayout *lay = new QVBoxLayout;
    lay->addWidget (Label_);
    setLayout (lay);
}
