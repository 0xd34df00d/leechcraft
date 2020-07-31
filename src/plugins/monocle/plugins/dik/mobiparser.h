/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QList>
#include <QFile>
#include <QHash>
#include <interfaces/monocle/idocument.h>

class QTextCodec;

namespace LC
{
namespace Monocle
{
namespace Dik
{
	class Decompressor;
	typedef std::shared_ptr<Decompressor> Decompressor_ptr;

	class MobiParser
	{
		bool IsValid_ = false;
		std::unique_ptr<QFile> File_;

		quint16 NumRecords_ = 0;
		QList<quint32> RecordOffsets_;

		Decompressor_ptr Dec_;

		bool IsDRM_ = false;

		quint16 TextRecordsCount_ = 0;
		quint16 MaxRecordSize_ = 0;

		quint16 FirstImgRec_ = 0;

		QTextCodec *Codec_ = 0;

		DocumentInfo DocInfo_;
	public:
		MobiParser (const QString&);

		bool IsValid () const;

		QByteArray GetRecord (int) const;

		QString GetText () const;

		int GetImageCount () const;
		QImage GetImage (int) const;

		const DocumentInfo& GetDocInfo () const;
	private:
		bool InitRecords ();
		bool InitHeader ();
		void ParseEXTH (const QByteArray&);

		void FindImageRecord ();

		template<typename T>
		void Read (T& t) const
		{
			File_->read (reinterpret_cast<char*> (&t), sizeof (T));
		}
	};
}
}
}
