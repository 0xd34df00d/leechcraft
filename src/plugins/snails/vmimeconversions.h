/**********************************************************************
	 *
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QString>
#include <QPair>
#include <vmime/mailbox.hpp>
#include <vmime/charsetConverter.hpp>
#include <vmime/utility/outputStreamStringAdapter.hpp>
#include "address.h"

class QIcon;
class QSslCertificate;

namespace vmime
{
class header;

namespace net
{
	class folder;
	class messageSet;
}
namespace security::cert
{
	class certificate;
}
}

namespace LC
{
namespace Snails
{
	enum class FolderType;

	template<typename T>
	QString Stringize (const T& t, const vmime::charset& source)
	{
		vmime::string stringized;
		vmime::utility::outputStreamStringAdapter out (stringized);
		t->extract (out);
		out.flush ();

		const auto& converter = vmime::charsetConverter::create (source, vmime::charsets::UTF_8);
		vmime::string outStr;
		converter->convert (stringized, outStr);

		return QString::fromStdString (outStr);
	}

	template<typename T>
	QString Stringize (const T& t)
	{
		vmime::string str;
		vmime::utility::outputStreamStringAdapter out (str);

		t->extract (out);

		return QString::fromStdString (str);
	}

	template<typename T>
	QString StringizeCT (const T& w)
	{
		return QString::fromStdString(w.getConvertedText (vmime::charsets::UTF_8));
	}

	inline Address Mailbox2Strings (const vmime::shared_ptr<const vmime::mailbox>& mbox)
	{
		if (!mbox)
			return {};

		return
		{
			StringizeCT (mbox->getName ()),
			QString::fromStdString (mbox->getEmail ().toString ())
		};
	}

	QStringList GetFolderPath (const vmime::shared_ptr<vmime::net::folder>&);
	vmime::net::messageSet ToMessageSet (const QList<QByteArray>&);

	QString GetFolderIconName (FolderType);
	QIcon GetFolderIcon (FolderType);

	QList<QSslCertificate> ToSslCerts (const vmime::shared_ptr<const vmime::security::cert::certificate>&);

	QByteArray SerializeHeader (const vmime::shared_ptr<const vmime::header>&);
}
}
