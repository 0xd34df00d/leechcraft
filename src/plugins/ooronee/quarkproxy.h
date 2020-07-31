/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <interfaces/iinfo.h>

namespace LC
{
namespace Ooronee
{
	class QuarkProxy : public QObject
	{
		Q_OBJECT

		const ICoreProxy_ptr Proxy_;

		struct VarInfo
		{
			QString HumanReadable_;

			QObject *Obj_;
			QByteArray Variant_;
		};

		Q_PROPERTY (int hoverTimeout READ GetHoverTimeout NOTIFY hoverTimeoutChanged)
	public:
		QuarkProxy (ICoreProxy_ptr);

		int GetHoverTimeout () const;
	private:
		void Handle (const QVariant&, const QByteArray&, bool);
		void HandleVariantsMenu (const QVariant&, const QByteArray&);
		void HandleVariantsDialog (Entity, const QStringList&, const QList<VarInfo>&, const QByteArray&);

		void SaveUsed (const QByteArray&, const QByteArray&, const QByteArray&);
	public slots:
		void handle (const QVariant&, bool);
	signals:
		void hoverTimeoutChanged ();
	};
}
}
