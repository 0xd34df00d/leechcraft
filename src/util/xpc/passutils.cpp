/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "passutils.h"
#include <QString>
#include <QObject>
#include <QInputDialog>
#include <util/xpc/util.h>
#include <interfaces/structures.h>
#include <interfaces/core/ientitymanager.h>
#include <interfaces/core/ipluginsmanager.h>
#include <interfaces/ipersistentstorageplugin.h>
#include <util/sll/eithercont.h>
#include <util/sll/slotclosure.h>

namespace LC
{
namespace Util
{
	namespace
	{
		QString GetPasswordHelper (const QByteArray& key, const ICoreProxy_ptr& proxy)
		{
			const auto& result = Util::GetPersistentData (key, proxy);
			if (!result.isValid ())
			{
				qWarning () << Q_FUNC_INFO
						<< "invalid result for key"
						<< key;
				return {};
			}

			switch (result.type ())
			{
			case QVariant::String:
				return result.toString ();
			case QVariant::List:
				return result.toList ().value (0).toString ();
			case QVariant::StringList:
				return result.toStringList ().value (0);
			default:
				qWarning () << Q_FUNC_INFO
						<< "unknown result type"
						<< result.type ()
						<< result
						<< "for key"
						<< key;
				return {};
			}
		}
	}

	QString GetPassword (const QString& key, const QString& diaText,
			const ICoreProxy_ptr& proxy, bool useStored)
	{
		if (useStored)
		{
			const auto& result = GetPasswordHelper (key.toUtf8 (), proxy);
			if (!result.isNull ())
				return result;
		}

		const auto& result = QInputDialog::getText (nullptr,
				"LeechCraft",
				diaText,
				QLineEdit::Password);
		if (!result.isNull ())
			SavePassword (result, key, proxy);
		return result;
	}

	void GetPassword (const QString& key, const QString& diaText,
			const ICoreProxy_ptr& proxy,
			const EitherCont<void (), void (QString)>& cont,
			QObject *depender,
			bool useStored)
	{
		if (useStored)
		{
			const auto& result = GetPasswordHelper (key.toUtf8 (), proxy);
			if (!result.isNull ())
			{
				cont.Right (result);
				return;
			}
		}

		const auto dialog = new QInputDialog;
		dialog->setInputMode (QInputDialog::TextInput);
		dialog->setWindowTitle ("LeechCraft");
		dialog->setLabelText (diaText);
		dialog->setTextEchoMode (QLineEdit::Password);
		dialog->setAttribute (Qt::WA_DeleteOnClose);

		if (depender)
			QObject::connect (depender,
					SIGNAL (destroyed ()),
					dialog,
					SLOT (deleteLater ()));

		new Util::SlotClosure<Util::DeleteLaterPolicy>
		{
			[dialog, cont]
			{
				const auto& value = dialog->textValue ();
				if (value.isEmpty ())
					cont.Left ();
				else
					cont.Right (value);
			},
			dialog,
			SIGNAL (accepted ()),
			dialog
		};

		new Util::SlotClosure<Util::DeleteLaterPolicy>
		{
			[cont] { cont.Left (); },
			dialog,
			SIGNAL (rejected ()),
			dialog
		};

		dialog->show ();
	}

	void SavePassword (const QString& password, const QString& key,
			const ICoreProxy_ptr& proxy)
	{
		const auto& plugins = proxy->GetPluginsManager ()->GetAllCastableTo<IPersistentStoragePlugin*> ();
		for (const auto plugin : plugins)
			if (const auto& storage = plugin->RequestStorage ())
				storage->Set (key.toUtf8 (), password);
	}
}
}
