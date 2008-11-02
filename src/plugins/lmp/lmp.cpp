#include "lmp.h"
#include <QToolBar>
#include <QSlider>
#include <QFileDialog>
#include <QDir>
#include <SeekSlider>
#include <VolumeSlider>
#include <plugininterface/util.h>
#include "xmlsettingsmanager.h"
#include "core.h"

void LMP::Init ()
{
	Translator_.reset (LeechCraft::Util::InstallTranslator ("lmp"));
	Ui_.setupUi (this);
	dynamic_cast<QBoxLayout*> (layout ())->
		insertWidget (0, SetupToolbar ());

	connect (&Core::Instance (),
			SIGNAL (stateUpdated (const QString&)),
			this,
			SLOT (handleStateUpdated (const QString&)));

	Core::Instance ().SetVideoWidget (Ui_.VideoWidget_);
	Core::Instance ().Reinitialize ();
}

void LMP::Release ()
{
	Core::Instance ().Release ();
}

QString LMP::GetName () const
{
	return "LMP";
}

QString LMP::GetInfo () const
{
	return "LeechCraft Media Player";
}

QStringList LMP::Provides () const
{
	return QStringList ("media");
}

QStringList LMP::Needs () const
{
	return QStringList ();
}

QStringList LMP::Uses () const
{
	return QStringList ();
}

void LMP::SetProvider (QObject*, const QString&)
{
}

QIcon LMP::GetIcon () const
{
	return QIcon ();
}

QWidget* LMP::GetTabContents ()
{
	return this;
}

QToolBar* LMP::SetupToolbar ()
{
	QToolBar *bar = new QToolBar;

	Open_.reset (new QAction (tr ("Open..."),
				this));
	Open_->setObjectName ("Open_");
	Open_->setProperty ("ActionIcon", "lmp_open");
	connect (Open_.get (),
			SIGNAL (triggered ()),
			this,
			SLOT (selectFile ()));

	Play_.reset (new QAction (tr ("Play"),
				this));
	Play_->setObjectName ("Play_");
	Play_->setProperty ("ActionIcon", "lmp_play");
	connect (Play_.get (),
			SIGNAL (triggered ()),
			&Core::Instance (),
			SLOT (play ()));

	Pause_.reset (new QAction (tr ("Pause"),
				this));
	Pause_->setObjectName ("Pause_");
	Pause_->setProperty ("ActionIcon", "lmp_pause");
	connect (Pause_.get (),
			SIGNAL (triggered ()),
			&Core::Instance (),
			SLOT (pause ()));

	Phonon::VolumeSlider *volumeSlider = new Phonon::VolumeSlider (this);
	Phonon::SeekSlider *seekSlider = new Phonon::SeekSlider (this);
	Core::Instance ().SetSeekSlider (seekSlider);
	Core::Instance ().SetVolumeSlider (volumeSlider);

	bar->addAction (Open_.get ());
	bar->addSeparator ();
	bar->addAction (Play_.get ());
	bar->addAction (Pause_.get ());
	bar->addSeparator ();
	bar->addWidget (volumeSlider);
	bar->addSeparator ();
	bar->addWidget (seekSlider);

	return bar;
}

bool LMP::ImplementsFeature (const QString& feature) const
{
	if (feature == "videoplayer" ||
			feature == "audioplayer")
		return true;
	else
		return false;
}

void LMP::handleStateUpdated (const QString& state)
{
	Ui_.State_->setText (state);
}

void LMP::setFile (const QString& file)
{
	Core::Instance ().setSource (file);
}

void LMP::play ()
{
	Core::Instance ().play ();
}

void LMP::selectFile ()
{
	QString oldDir = XmlSettingsManager::Instance ()->
		Property ("LastOpenFileName", QDir::homePath ()).toString ();

	QString filename = QFileDialog::getOpenFileName (this,
			tr ("Select media file"),
			oldDir,
			tr ("Video (*.avi *.mkv *.ogg *.mpeg *.mpg *.divx *.mov *.swf);;" "Uncompressed lossless audio (*.aiff *.au *.cdda *.raw *.wav);;"
				"Compressed lossless audio (*.flac *.wv *.m4a *.tta *.ape *.la *.pac);;"
				"Lossy audio (*.mp3 *.ogg *.wma *.aac *.mpc *.ra *.rm *.ots *.swa);;"
				"All files (*.*)"));

	if (filename.isEmpty ())
		return;

	XmlSettingsManager::Instance ()->setProperty ("LastOpenFileName",
			QDir (filename).absolutePath ());
	Core::Instance ().setSource (filename);
}

Q_EXPORT_PLUGIN2 (leechcraft_lmp, LMP);

