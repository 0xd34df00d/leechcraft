#ifndef CORE_H
#define CORE_H
#include <memory>
#include <QObject>
#include <MediaObject>
#include <AudioOutput>

namespace Phonon
{
	class VideoWidget;
	class SeekSlider;
	class VolumeSlider;
};

class Core : public QObject
{
	Q_OBJECT

	std::auto_ptr<Phonon::MediaObject> MediaObject_;
	std::auto_ptr<Phonon::AudioOutput> AudioOutput_;

	bool TotalTimeAvailable_;

	Phonon::VideoWidget *VideoWidget_;
	Phonon::SeekSlider *SeekSlider_;
	Phonon::VolumeSlider *VolumeSlider_;

	Core ();
public:
	static Core& Instance ();
	void Release ();

	void Reinitialize ();
	Phonon::MediaObject* GetMediaObject () const;
	void SetVideoWidget (Phonon::VideoWidget*);
	void SetSeekSlider (Phonon::SeekSlider*);
	void SetVolumeSlider (Phonon::VolumeSlider*);
public slots:
	void play ();
	void pause ();
	void setSource (const QString&);
private slots:
	void updateState ();
	void totalTimeChanged ();
	void handleHasVideoChanged (bool);
signals:
	void stateUpdated (const QString&);
};

#endif

