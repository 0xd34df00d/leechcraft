/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2011  Andrey Batyiev
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QLabel>

namespace LC::Azoth::Xoox
{
	class XMPPBobManager;
	class XMPPBobIq;

	class ImageMediaWidget : public QLabel
	{
	public:
		ImageMediaWidget (const QUrl&, XMPPBobManager*, const QString&, QWidget* = nullptr);
	};
}

