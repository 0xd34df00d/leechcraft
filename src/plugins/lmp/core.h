#ifndef CORE_H
#define CORE_H
#include <memory>
#include <QObject>
#include <MediaObject>
#include <AudioOutput>

namespace Phonon
{
	class VideoWidget;
};

class Core : public QObject
{
	Q_OBJECT

	std::auto_ptr<Phonon::MediaObject> MediaObject_;
	std::auto_ptr<Phonon::AudioOutput> AudioOutput_;

	bool TotalTimeAvailable_;

	Phonon::VideoWidget *VideoWidget_;

	Core ();
public:
	static Core& Instance ();
	void Release ();

	void Reinitialize (const QString&);
	Phonon::MediaObject* GetMediaObject () const;
	void SetVideoWidget (Phonon::VideoWidget*);
public slots:
	void changeVolume (int);
	void play ();
	void pause ();
	void setSource (const QString&);
private slots:
	void updateState ();
	void totalTimeChanged ();
signals:
	void stateUpdated (const QString&);
};

#endif

