/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "requestbuilder.h"
#include <QUuid>

namespace LC::Imgaste
{
	RequestBuilder::RequestBuilder ()
	{
		QString rnd = QUuid::createUuid ().toString ();
		rnd = rnd.mid (1, rnd.size () - 2);
		rnd += rnd;
		rnd = rnd.left (55);

		Boundary_ = "----------";
		Boundary_ += std::move (rnd).toLatin1 ();
	}

	void RequestBuilder::AddPair (const QString& name, const QString& value)
	{
		Result_ += "--";
		Result_ += Boundary_;
		Result_ += "\r\n";
		Result_ += "Content-Disposition: form-data; name=\"";
		Result_ += name.toLatin1 ();
		Result_ += "\"";
		Result_ += "\r\n\r\n";
		Result_ += value.toUtf8 ();
		Result_ += "\r\n";
	}

	void RequestBuilder::AddFile (const QString& format,
			const QString& name, const QByteArray& imageData)
	{
		Result_ += "--";
		Result_ += Boundary_;
		Result_ += "\r\n";
		Result_ += "Content-Disposition: form-data; name=\"";
		Result_ += name.toLatin1 ();
		Result_ += "\"; ";
		Result_ += "filename=\"";
		Result_ += QString ("screenshot.%1")
			.arg (format.toLower ())
			.toLatin1 ();
		Result_ += "\"";
		Result_ += "\r\n";
		Result_ += "Content-Type: ";
		if (format.toLower () == "jpg")
			Result_ += "image/jpeg";
		else
			Result_ += "image/png";
		Result_ += "\r\n\r\n";

		Result_ += imageData;
		Result_ += "\r\n";
	}

	namespace
	{
		static const QByteArray Separator { "--" };
	}

	QByteArray RequestBuilder::Build () const
	{
		return Result_ + Separator + Boundary_ + Separator;
	}

	int RequestBuilder::Size () const
	{
		return Result_.size () + Boundary_.size () + 2 * Separator.size ();
	}

	QString RequestBuilder::GetBoundary () const
	{
		return Boundary_;
	}
}
