/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "vmimeconversions.h"
#include <QStringList>
#include <QIcon>
#include <QBuffer>
#include <vmime/net/folder.hpp>
#include <vmime/security/cert/certificate.hpp>
#include <interfaces/core/iiconthememanager.h>
#include "outputiodevadapter.h"
#include "folder.h"
#include "core.h"

namespace LC
{
namespace Snails
{
	Address Mailbox2Strings (const vmime::shared_ptr<const vmime::mailbox>& mbox)
	{
		if (!mbox)
			return {};

		return
		{
			StringizeCT (mbox->getName ()),
			QString::fromStdString (mbox->getEmail ().toString ())
		};
	}

	QStringList GetFolderPath (const vmime::shared_ptr<vmime::net::folder>& folder)
	{
		QStringList pathList;
		const auto& path = folder->getFullPath ();
		for (size_t i = 0; i < path.getSize (); ++i)
			pathList << StringizeCT (path.getComponentAt (i));
		return pathList;
	}

	vmime::net::messageSet ToMessageSet (const QList<QByteArray>& ids)
	{
		auto set = vmime::net::messageSet::empty ();
		for (const auto& id : ids)
			set.addRange (vmime::net::UIDMessageRange (id.constData ()));
		return set;
	}

	QString GetFolderIconName (FolderType type)
	{
		switch (type)
		{
		case FolderType::Inbox:
			return "mail-folder-inbox";
		case FolderType::Drafts:
			return "mail-folder-outbox";
		case FolderType::Sent:
			return "mail-folder-sent";
		case FolderType::Important:
			return "mail-mark-important";
		case FolderType::Junk:
			return "mail-mark-junk";
		case FolderType::Trash:
			return "user-trash";
		default:
			return "folder-documents";
		}
	}

	QIcon GetFolderIcon (FolderType type)
	{
		return Core::Instance ().GetProxy ()->GetIconThemeManager ()->GetIcon (GetFolderIconName (type));
	}

	QList<QSslCertificate> ToSslCerts (const vmime::shared_ptr<const vmime::security::cert::certificate>& cert)
	{
		if (!cert)
			return {};

		const auto& encoded = cert->getEncoded ();
		const auto& data = QByteArray::fromRawData (reinterpret_cast<const char*> (&encoded [0]),
				static_cast<int> (encoded.size ()));
		const auto& qCerts = QSslCertificate::fromData (data, QSsl::Der);
		if (!qCerts.isEmpty ())
			return qCerts;

		qDebug () << Q_FUNC_INFO
				<< "retrying with PEM for type"
				<< cert->getType ().c_str ();
		return QSslCertificate::fromData (data, QSsl::Pem);
	}

	QByteArray SerializeHeader (const vmime::shared_ptr<const vmime::header>& header)
	{
		QBuffer buffer;
		buffer.open (QIODevice::WriteOnly);
		OutputIODevAdapter adapter { &buffer };
		header->generate (adapter);
		return buffer.buffer ();
	}
}
}
