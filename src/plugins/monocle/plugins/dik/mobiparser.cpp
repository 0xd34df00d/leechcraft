/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "mobiparser.h"
#include <QFile>
#include <QtDebug>
#include <QtEndian>
#include <QBuffer>
#include <QImageReader>
#include "decompressor.h"
#include "util.h"

namespace LC
{
namespace Monocle
{
namespace Dik
{
	MobiParser::MobiParser (const QString& filename)
	: File_ (new QFile (filename))
	{
		if (!File_->open (QIODevice::ReadOnly))
		{
			qWarning () << Q_FUNC_INFO
					<< "cannot open file"
					<< File_->errorString ();
			return;
		}

		IsValid_ = InitRecords () && InitHeader ();
		FindImageRecord ();
	}

	bool MobiParser::IsValid () const
	{
		return IsValid_;
	}

	QByteArray MobiParser::GetRecord (int idx) const
	{
		if (idx >= NumRecords_)
			return {};

		const auto offset = RecordOffsets_.at (idx);
		if (!File_->seek (offset))
			return {};

		return idx == NumRecords_ - 1 ?
				File_->readAll () :
				File_->read (RecordOffsets_.at (idx + 1) - offset);
	}

	QString MobiParser::GetText () const
	{
		QByteArray result;
		for (quint32 i = 1; i <= TextRecordsCount_; ++i)
		{
			QByteArray decompressed;
			try
			{
				decompressed = (*Dec_) (GetRecord (i));
			}
			catch (const std::exception& e)
			{
				qWarning () << Q_FUNC_INFO
						<< "error decompressing file"
						<< e.what ();
				throw;
			}

			if (decompressed.size () > MaxRecordSize_)
				decompressed.resize (MaxRecordSize_);

			result += decompressed;
		}
		return Codec_->decode (result);
	}

	int MobiParser::GetImageCount () const
	{
		return NumRecords_ - TextRecordsCount_;
	}

	QImage MobiParser::GetImage (int imgIdx) const
	{
		return QImage::fromData (GetRecord (imgIdx + FirstImgRec_));
	}

	const DocumentInfo& MobiParser::GetDocInfo () const
	{
		return DocInfo_;
	}

	bool MobiParser::InitRecords ()
	{
		if (!File_->seek (0x3c))
			return false;

		const QString filetype (File_->read (8));

		if (!File_->seek (0x4c))
			return false;

		Read (NumRecords_);
		NumRecords_ = qFromBigEndian (NumRecords_);

		for (quint16 i = 0; i < NumRecords_; ++i)
		{
			quint32 offset = 0;
			Read (offset);
			RecordOffsets_ << qFromBigEndian (offset);
			Read (offset);
		}

		return true;
	}

	namespace
	{
		Decompressor::Type ToType (quint8 c)
		{
			switch (c)
			{
			case 1:
				return Decompressor::Type::None;
			case 2:
				return Decompressor::Type::RLE;
			case 'H':
				return Decompressor::Type::Huff;
			}

			qWarning () << Q_FUNC_INFO
					<< "unknown compression type"
					<< c;
			return Decompressor::Type::None;
		}
	}

	bool MobiParser::InitHeader ()
	{
		const auto& headrec = GetRecord (0);
		if (headrec.size () < 14)
			return false;

		try
		{
			if (!(Dec_ = Decompressor::Create (ToType (headrec [1]), this)))
				return false;
		}
		catch (const std::exception& e)
		{
			qWarning () << Q_FUNC_INFO
					<< "error creating decompressor:"
					<< e.what ();
			return false;
		}

		IsDRM_ = headrec [12] || headrec [13];

		auto twoBytes = [&headrec] (int pos)
		{
			return (static_cast<quint16> (headrec [pos]) << 8) +
					static_cast<uchar> (headrec [pos + 1]);
		};

		TextRecordsCount_ = twoBytes (8);
		MaxRecordSize_ = twoBytes (10);

		const auto isUnicode = headrec.size () > 31 && Read32 (headrec, 28) == 65001;
		Codec_ = std::make_unique<QStringDecoder> (isUnicode ? "UTF-8" : "CP1252");

		if (headrec.size () > 176)
			ParseEXTH (headrec);

		return true;
	}

	namespace
	{
		QString ReadEXTHField (const QByteArray& data, quint32& offset, QStringDecoder& codec)
		{
			auto len = Read32 (data, offset);
			offset += 4;
			len -= 8;

			const auto& result = codec.decode (data.mid (offset, len));
			offset += len;
			return result;
		}
	}

	void MobiParser::ParseEXTH (const QByteArray& rec)
	{
		if (rec.size () >= 92)
		{
			const auto nameOffset = Read32 (rec, 0x54);
			const auto nameLen = Read32 (rec, 0x58);
			if (nameOffset + nameLen < static_cast<quint32> (rec.size ()))
				DocInfo_.Title_ = Codec_->decode (rec.mid (nameOffset, nameLen));
		}

		const auto exthOff = Read32 (rec, 0x14) + 16;
		if (rec.mid (exthOff, 4) != "EXTH")
			return;

		const auto recs = Read32 (rec, exthOff + 8);
		auto offset = exthOff + 12;
		for (quint32 i = 0; i < recs; ++i)
		{
			if (offset + 4 > static_cast<quint32> (rec.size ()))
				break;

			const auto type = Read32 (rec, offset);
			offset += 4;
			switch (type)
			{
			case 100:
				DocInfo_.Author_ = ReadEXTHField (rec, offset, *Codec_);
				break;
			case 103:
				DocInfo_.Description_ = ReadEXTHField (rec, offset, *Codec_);
				break;
			case 105:
				DocInfo_.Subject_ = ReadEXTHField (rec, offset, *Codec_);
				break;
			case 109:
				DocInfo_.Author_ = ReadEXTHField (rec, offset, *Codec_);
				break;
			}
		}
	}

	void MobiParser::FindImageRecord ()
	{
		if (FirstImgRec_)
			return;

		for (FirstImgRec_ = TextRecordsCount_ + 1; FirstImgRec_ < NumRecords_; ++FirstImgRec_)
		{
			auto rec = GetRecord (FirstImgRec_);
			if (rec.isNull ())
				return;

			QBuffer buf (&rec);
			buf.open (QIODevice::ReadOnly);
			if (QImageReader (&buf).canRead ())
				break;
		}
	}
}
}
}
