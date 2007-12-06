#ifndef METAINFO_H
#define METAINFO_H
#include <QByteArray>
#include <QString>
#include <QList>
#include <QDateTime>
#include <QStringList>

struct MetaInfoSingleFile
{
	qint64 Length_;
	QByteArray MD5Sum_;
	QString Name_;
	int PieceLength_;
	QList<QByteArray> Sha1Sums_;
};

struct MetaInfoMultifile
{
	qint64 Length_;
	QByteArray MD5Sum_;
	QString Path_;
};

class MetaInfo
{
	QString ErrorString_;
	QByteArray Content_;
	QByteArray Info_;

	MetaInfoSingleFile MetaInfoSingleFile_;
	QList<MetaInfoMultifile> MetaInfoMultifile_;
	QString MetaAnnounce_;
	QStringList MetaAnnounceList_;
	QDateTime MetaCreationDate_;
	QString MetaComment_;
	QString MetaCreatedBy_;
	QString MetaName_;
	int MetaPieceLength_;
	QList<QByteArray> MetaSha1Sums_;
public:
	enum Form
	{
		FormSingle
		, FormMulti
	};
private:
	Form FileForm_;
public:
	MetaInfo ();

	void Clear ();
	bool Parse (const QByteArray&);

	QString GetErrorString () const;
	QByteArray GetInfo () const;

	Form GetForm () const;
	QString GetAnnounceURL () const;
	QStringList GetAnnounceList () const;
	QDateTime GetCreationDate () const;
	QString GetComment () const;
	QString GetCreatedBy () const;

	MetaInfoSingleFile GetSingleFileInfo () const;

	QList<MetaInfoMultifile> GetMultiFilesInfo () const;
	QString GetName () const;
	int GetPieceLength () const;
	QList<QByteArray> GetSha1Sums () const;

	qint64 GetTotalSize () const;
};

#endif

