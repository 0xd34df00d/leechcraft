/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "util.h"
#include <QString>
#include <QStringList>
#include <QByteArray>
#include <QFileDialog>
#include <QDir>
#include <interfaces/core/ientitymanager.h>
#include <util/threads/futures.h>
#include <util/gui/util.h>
#include <util/sll/visitor.h>
#include <util/xpc/util.h>
#include "account.h"

namespace LC::Snails
{
	QString PlainBody2HTML (const QString& body)
	{
		auto str = body.toHtmlEscaped ();
		str.replace ("\r\n", "<br/>");
		str.replace ("\r", "<br/>");
		str.replace ("\n", "<br/>");
		return str;
	}

	void RunAttachmentSaveDialog (Account *acc, IEntityManager *iem,
			const QByteArray& id, const QStringList& folder, const QString& name)
	{
		const auto& path = QFileDialog::getSaveFileName (nullptr,
				QObject::tr ("Save attachment"),
				QDir::homePath () + '/' + name);
		if (path.isEmpty ())
			return;

		Util::Sequence (nullptr, acc->FetchAttachment (folder, id, name, path)) >>
				Util::Visitor
				{
					[iem, name] (Util::Void)
					{
						iem->HandleEntity (Util::MakeNotification ("LeechCraft Snails",
								QObject::tr ("Attachment %1 fetched successfully.")
										.arg (Util::FormatName (name)),
								Priority::Info));
					},
					[iem, name] (auto errVar)
					{
						iem->HandleEntity (Util::MakeNotification ("LeechCraft Snails",
								QObject::tr ("Unable to fetch %1: %2.")
										.arg (Util::FormatName (name))
										.arg (Util::Visit (errVar, [] (auto err) { return err.what (); })),
								Priority::Critical));
					}
				};
	}
}
