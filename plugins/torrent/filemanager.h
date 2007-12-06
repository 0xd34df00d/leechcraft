#ifndef FILEMANAGER_H
#define FILEMANAGER_H
#include <QBitArray>
#include <QList>
#include <QMutex>
#include <QThread>
#include <QWaitCondition>
#include "metainfo.h"

class QByteArray;
class QFile;
class QTimerEvent;

class FileManager : public QThread
{
	Q_OBJECT
	
	MetaInfo MetaInfo_;
	QString DestinationPath_, ErrorString_;
	qint64 TotalLength_;
	int NumPieces_;
	QList<QFile*> Files_;
	QList<QByteArray> Sha1s_;
	QBitArray VerifiedPieces_;
	int PieceLength_, ReadID_;
	bool NewFile_, StartVerification_, Quit_, WokeUp_;
	QList<int> PendingVerificationRequests_, NewPendingVerificationRequests_;
	QList<qint64> FileSizes_;
	mutable QMutex Mutex_;
	mutable QWaitCondition Condition_;

	struct WriteRequest
	{
		int Index_;
		int Offset_;
		QByteArray Data_;
	};

	struct ReadRequest
	{
		int ID_;
		int Index_;
		int Offset_;
		int Length_;
	};
	QList<WriteRequest> WriteRequests_;
	QList<ReadRequest> ReadRequests_;
public:
	FileManager (QObject *parent = 0);
	virtual ~FileManager ();

	void SetMetaInfo (const MetaInfo&);
	void SetDestinationFolder (const QString&);

	int Read (int, int, int);
	void Write (int, int, const QByteArray&);
	void VerifyPiece (int);
	qint64 GetTotalSize () const;
	int GetPieceCount () const;
	int GetPieceLengthAt (int) const;
	const QBitArray& GetCompletedPieces () const;
	void SetCompletedPieces (const QBitArray&);
	const QString& GetErrorString () const;
public slots:
	void startDataVerification ();
signals:
	void dataRead (int, int, int, const QByteArray&);
	void error ();
	void verificationProgress (int);
	void verificationDone ();
	void pieceVerified (int, bool);
protected:
	virtual void run ();
private:
	bool GenerateFiles ();
	QByteArray ReadBlock (int, int, int);
	bool WriteBlock (int, int, const QByteArray&);
	void VerifyFileContents ();
private slots:
	bool verifySinglePiece (int);
	void wakeUp ();
};

#endif

