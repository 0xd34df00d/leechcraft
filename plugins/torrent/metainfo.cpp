#include "metainfo.h"
#include "bencodeparser.h"

namespace
{
	const int PieceHashLength = 20;
};

MetaInfo::MetaInfo ()
{
	Clear ();
}

void MetaInfo::Clear ()
{
	ErrorString_ = QObject::tr ("Unknown error");
	Content_.clear ();
	Info_.clear ();
	MetaInfoMultifile_.clear ();
	MetaAnnounce_.clear ();
	MetaAnnounceList_.clear ();
	MetaCreationDate_ = QDateTime ();
	MetaComment_.clear ();
	MetaCreatedBy_.clear ();
	MetaName_.clear ();
	MetaPieceLength_ = 0;
	MetaSha1Sums_.clear ();
}

bool MetaInfo::Parse (const QByteArray& data)
{
	Clear ();
	Content_ = data;

	BencodeParser parser;
	if (!parser.Parse (Content_))
	{
		ErrorString_ = parser.GetErrorString ();
		return false;
	}
	Info_ = parser.GetInfoSection ();

	QMap<QByteArray, QVariant> dict = parser.GetParsed ();
	if (!dict.contains ("info"))
		return false;

	Dictionary_t info = qVariantValue<Dictionary_t> (dict ["info"]);

	if (info.contains ("files"))
	{
		FileForm_ = FormMulti;

		QList<QVariant> files = info ["files"].toList ();

		for (int i = 0; i < files.size (); ++i)
		{
			Dictionary_t file = qVariantValue<Dictionary_t> (files [i]);
			QList<QVariant> paths = file ["path"].toList ();
			QByteArray path;
			foreach (QVariant p, paths)
			{
				if (!path.isEmpty ())
					path += "/";
				path += p.toByteArray ();
			}

			MetaInfoMultifile mf;
			mf.Length_ = file ["length"].toLongLong ();
			mf.Path_ = QString::fromUtf8 (path);
			mf.MD5Sum_ = file ["md5sum"].toByteArray ();
			MetaInfoMultifile_ << mf;
		}

		MetaName_ = QString::fromUtf8 (info ["name"].toByteArray ());
		MetaPieceLength_ = info ["piece length"].toInt ();
		QByteArray Pieces_ = info ["pieces"].toByteArray ();
		for (int i = 0; i < Pieces_.size (); i += PieceHashLength)
			MetaSha1Sums_ << Pieces_.mid (i, PieceHashLength);
	}
	else if (info.contains ("length"))
	{
		FileForm_ = FormSingle;
		MetaInfoSingleFile_.Length_ = info ["length"].toLongLong ();
		MetaInfoSingleFile_.MD5Sum_ = info ["md5sum"].toByteArray ();
		MetaInfoSingleFile_.Name_ = QString::fromUtf8 (info ["name"].toByteArray ());
		MetaInfoSingleFile_.PieceLength_ = info ["piece length"].toInt ();

		QByteArray pieces = info ["pieces"].toByteArray ();
		for (int i = 0; i < pieces.size (); i += PieceHashLength)
			MetaInfoSingleFile_.Sha1Sums_ << pieces.mid (i, PieceHashLength);
	}

	MetaAnnounce_ = QString::fromUtf8 (dict ["announce"].toByteArray ());

	if (dict.contains ("creation date"))
		MetaCreationDate_.setTime_t (dict ["creation date"].toInt ());
	if (dict.contains ("comment"))
		MetaComment_ = QString::fromUtf8 (dict ["comment"].toByteArray ());
	if (dict.contains ("created by"))
		MetaCreatedBy_ = QString::fromUtf8 (dict ["created by"].toByteArray ());

	return true;
}

QString MetaInfo::GetErrorString () const
{
	return ErrorString_;
}

QByteArray MetaInfo::GetInfo () const
{
	return Info_;
}

MetaInfo::Form MetaInfo::GetForm () const
{
	return FileForm_;
}

QString MetaInfo::GetAnnounceURL () const
{
	return MetaAnnounce_;
}

QStringList MetaInfo::GetAnnounceList () const
{
	return MetaAnnounceList_;
}

QDateTime MetaInfo::GetCreationDate () const
{
	return MetaCreationDate_;
}

QString MetaInfo::GetComment () const
{
	return MetaComment_;
}

QString MetaInfo::GetCreatedBy () const
{
	return MetaCreatedBy_;
}

MetaInfoSingleFile MetaInfo::GetSingleFileInfo () const
{
	return MetaInfoSingleFile_;
}

QList<MetaInfoMultifile> MetaInfo::GetMultiFilesInfo () const
{
	return MetaInfoMultifile_;
}

QString MetaInfo::GetName () const
{
	return MetaName_;
}

int MetaInfo::GetPieceLength () const
{
	return MetaPieceLength_;
}

QList<QByteArray> MetaInfo::GetSha1Sums () const
{
	return MetaSha1Sums_;
}

qint64 MetaInfo::GetTotalSize () const
{
	if (GetForm () == FormSingle)
		return GetSingleFileInfo ().Length_;

	qint64 result = 0;
	foreach (MetaInfoMultifile file, GetMultiFilesInfo ())
		result += file.Length_;
	return result;
}

