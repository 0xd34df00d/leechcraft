#ifndef RATECONTROLLER_H
#define RATECONTROLLER_H
#include <QObject>
#include <QSet>
#include <QTime>

class PeerConnection;

class RateController : public QObject
{
	Q_OBJECT

	QTime StopWatch_;
	QSet<PeerConnection*> Sockets_;
	int UpLimit_;
	int DownLimit_;
	bool TransferScheduled_;
public:
	RateController (QObject *parent = 0);
	static RateController* Instance ();

	void AddSocket (PeerConnection*);
	void RemoveSocket (PeerConnection*);

	int GetUploadLimit () const;
	int GetDownloadLimit () const;
	void SetUploadLimit (int);
	void SetDownloadLimit (int);
public slots:
	void transfer ();
	void scheduleTransfer ();
};

#endif

