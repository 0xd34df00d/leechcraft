/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QCoreApplication>
#include <QString>
#include <interfaces/idatafilter.h>

class QStandardItemModel;

namespace LC::Imgaste
{
	enum class Format;

	class Uploader : public QObject
	{
		Q_DECLARE_TR_FUNCTIONS (LC::Imgaste::Uploader)

		const QByteArray Data_;
		const Format Format_;
		const DataFilterCallback_f Callback_;

		QStandardItemModel * const ReprModel_;
	public:
		explicit Uploader (QByteArray data,
				Format format,
				DataFilterCallback_f callback,
				QStandardItemModel *reprModel);

		void Upload (const QString& service);
	private:
		void TryAnotherService (const QString&);
	};
}
