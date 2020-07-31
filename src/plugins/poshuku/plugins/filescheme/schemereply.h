/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QNetworkReply>
#include <QBuffer>

namespace LC
{
namespace Poshuku
{
namespace FileScheme
{
	class SchemeReply final : public QNetworkReply
	{
		QBuffer Buffer_;
	public:
		SchemeReply (const QNetworkRequest&, QObject* = 0);
		virtual ~SchemeReply ();

		qint64 bytesAvailable () const override;
		void abort () override;
		void close () override;
	protected:
		qint64 readData (char*, qint64) override;
	private:
		void List ();
	};
}
}
}
