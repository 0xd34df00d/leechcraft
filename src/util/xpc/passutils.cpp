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
#include <util/sll/qtutil.h>

namespace LC::Util
{
	namespace
	{
		QString GetPasswordHelper (const QByteArray& key, const ICoreProxy_ptr& proxy)
		{
			const auto& result = GetPersistentData (key, proxy);
			if (!result.isValid ())
			{
				qWarning () << "invalid result for key" << key;
				return {};
			}

			return HandleQVariant (result,
					[] (const QString& str) { return str; },
					[] (const QVariantList& list) { return list.value (0).toString (); },
					[] (const QStringList& list) { return list.value (0); },
					[&]
					{
						qWarning () << "unknown result type" << result.metaType () << result << "for key" << key;
						return QString {};
					});
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
				QStringLiteral ("LeechCraft"),
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
		dialog->setWindowTitle (QStringLiteral ("LeechCraft"));
		dialog->setLabelText (diaText);
		dialog->setTextEchoMode (QLineEdit::Password);
		dialog->setAttribute (Qt::WA_DeleteOnClose);

		if (depender)
			QObject::connect (depender,
					&QObject::destroyed,
					dialog,
					&QObject::deleteLater);

		QObject::connect (dialog,
				&QDialog::finished,
				[dialog, cont] (int r)
				{
					const auto& value = dialog->textValue ();
					if (r == QDialog::Rejected || value.isEmpty ())
						cont.Left ();
					else
						cont.Right (value);
				});

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
