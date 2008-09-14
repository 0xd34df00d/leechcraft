#include <QtGui/QtGui>
#include <interfaces/structures.h>
#include <plugininterface/util.h>
#include "batcher.h"
#include "parser.h"
#include "globals.h"

Batcher::~Batcher ()
{
}

void Batcher::Init ()
{
	Translator_.reset (LeechCraft::Util::InstallTranslator ("batcher"));

    IsShown_ = false;
    Parser_.reset (new Parser (this));

    setupUi (this);

    connect (Step_, SIGNAL (valueChanged (int)), this, SLOT (collectDataAndParse ()));
    connect (LowerBoundary_, SIGNAL (valueChanged (int)), this, SLOT (collectDataAndParse ()));
    connect (UpperBoundary_, SIGNAL (valueChanged (int)), this, SLOT (collectDataAndParse ()));
    connect (PatternLine_, SIGNAL (textChanged (const QString&)), this, SLOT (collectDataAndParse ()));
    connect (LocalDirLine_, SIGNAL (textChanged (const QString&)), this, SLOT (collectDataAndParse ()));
    connect (LeadingZeroes_, SIGNAL (stateChanged (int)), this, SLOT (collectDataAndParse ()));
    connect (OK_, SIGNAL (released ()), this, SLOT (sendJobs ()));
}

QString Batcher::GetName () const
{
    return Globals::Name;
}

QString Batcher::GetInfo () const
{
    return tr ("Batch job adder.");
}

QString Batcher::GetStatusbarMessage () const
{
    return "";
}

IInfo& Batcher::SetID (IInfo::ID_t id)
{
    ID_ = id;
    return *this;
}

IInfo::ID_t Batcher::GetID () const
{
    return ID_;
}

QStringList Batcher::Provides () const
{
    return QStringList ();
}

QStringList Batcher::Needs () const
{
    return QStringList ("http") << "ftp";
}

QStringList Batcher::Uses () const
{
    return QStringList ();
}

void Batcher::SetProvider (QObject *obj, const QString& feature)
{
    Providers_ [feature] = obj;
}

void Batcher::PushMainWindowExternals (const MainWindowExternals&)
{
}

void Batcher::Release ()
{
}

QIcon Batcher::GetIcon () const
{
    return QIcon (":/resources/images/batcher.png");
}

void Batcher::SetParent (QWidget *parent)
{
    setParent (parent);
}

void Batcher::ShowWindow ()
{
    IsShown_ = 1 - IsShown_;
    IsShown_ ? show () : hide ();
}

void Batcher::ShowBalloonTip ()
{
}

void Batcher::handleHidePlugins ()
{
    IsShown_ = false;
    hide ();
}

void Batcher::closeEvent (QCloseEvent *e)
{
    e->accept ();
    IsShown_ = false;
}

void Batcher::collectDataAndParse ()
{
    UpperBoundary_->setMinimum (LowerBoundary_->value () + 1);
    Parser::ParserData pd = { PatternLine_->text (), LowerBoundary_->value (), UpperBoundary_->value (), Step_->value (), LeadingZeroes_->checkState () == Qt::Checked};
    QStringList result = Parser_->Parse (pd);
    if (result.size () && LocalDirLine_->text ().size ())
        OK_->setEnabled (true);
    else
        OK_->setEnabled (false);

    PreviewList_->clear ();
    PreviewList_->addItems (result);
}

void Batcher::sendJobs ()
{
    int count = 0;
    QObject *provider = 0;
    QString localDir = LocalDirLine_->text ();
    if (!QFileInfo (localDir).exists ())
    {
        localDir = QDir::toNativeSeparators (QDir::homePath ());
        LocalDirLine_->setText (localDir);
        QMessageBox::warning (this, tr ("Warning"), tr ("Because the specified directory doesn't exist, files will be downloaded to %1.").arg (localDir));
    }
    if (!localDir.endsWith ("/"))
        localDir.append ("/");
    for (int i = 0; i < PreviewList_->count (); ++i)
    {
        QString text = PreviewList_->item (i)->text ();
        if (text.left (6).toLower () == "ftp://")
            provider = Providers_ ["ftp"];
        else if (text.left (7).toLower () == "http://")
            provider = Providers_ ["http"];
        else
            continue;
        if (!provider)
            continue;
        DirectDownloadParams ddd = { text, localDir + QFileInfo (text).fileName () };
        IDirectDownload *idd = qobject_cast<IDirectDownload*> (provider);
		idd->AddJob (ddd, LeechCraft::Autostart);
        ++count;
    }
    if (count)
        QMessageBox::information (this, tr ("Finished"), tr ("%1 jobs successfully added.").arg (count));
    else
        QMessageBox::warning (this, tr ("Finished"), tr ("No jobs were added. Check your pattern string or boundaries."));
}

void Batcher::on_LocalBrowse__released ()
{
    QString dir = QFileDialog::getExistingDirectory (this, tr ("Select base path"), QDir::homePath ());
    if (dir.isEmpty ())
        return;
    else
        LocalDirLine_->setText (dir);
}

Q_EXPORT_PLUGIN2 (leechcraft_batcher, Batcher);

