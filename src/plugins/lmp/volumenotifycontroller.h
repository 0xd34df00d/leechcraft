/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>

class QTimer;

namespace LC
{
namespace LMP
{
	class Output;

	class VolumeNotifyController : public QObject
	{
		Q_OBJECT

		Output * const Output_;
		QTimer * const NotifyTimer_;
	public:
		VolumeNotifyController (Output*, QObject* = 0);
	public slots:
		void volumeUp ();
		void volumeDown ();
	private slots:
		void notify ();
	};
}
}
