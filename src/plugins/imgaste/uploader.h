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

namespace LC
{
	struct Entity;
}

namespace LC::Imgaste
{
	enum class Format;

	void Upload (const QByteArray&, QSize, const Entity&, Format, QStandardItemModel*);
}
