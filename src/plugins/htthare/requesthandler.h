/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <boost/asio/buffer.hpp>
#include <QByteArray>
#include <QUrl>
#include <QMap>
#include <QCoreApplication>

class QFileInfo;

namespace LC
{
namespace HttHare
{
	class Connection;
	typedef std::shared_ptr<Connection> Connection_ptr;

	class RequestHandler
	{
		Q_DECLARE_TR_FUNCTIONS (LC::HttHare::RequestHandler)

		const Connection_ptr Conn_;

		QUrl Url_;
		QMap<QString, QString> Headers_;

		QByteArray ResponseLine_;
		QList<QPair<QByteArray, QByteArray>> ResponseHeaders_;
		QByteArray CookedRH_;
		QByteArray ResponseBody_;

		enum class Verb
		{
			Get,
			Head
		};
	public:
		RequestHandler (const Connection_ptr&);

		void operator() (QByteArray);
	private:
		QString Tr (const char*);

		void ErrorResponse (int, const QByteArray&, const QByteArray& = QByteArray ());
		QByteArray MakeDirResponse (const QFileInfo&, const QString&, const QUrl&);

		void HandleRequest (Verb);
		void WriteDir (const QString&, const QFileInfo&, Verb);
		void WriteFile (const QString&, const QFileInfo&, Verb);
		void DefaultWrite (Verb);
		std::vector<boost::asio::const_buffer> ToBuffers (Verb);
	};
}
}
