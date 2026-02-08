/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QLabel>
#include "guiconfig.h"

namespace LC::Util
{
	class UTIL_GUI_API ElidingLabel : public QLabel
	{
		QString FullText_;
		Qt::TextElideMode ElideMode_ = Qt::ElideMiddle;
	public:
		explicit ElidingLabel (QWidget* = nullptr);

		void SetFullText (const QString&);
		QString GetFullText () const;

		void SetElideMode (Qt::TextElideMode);
		Qt::TextElideMode GetElideMode () const;
	protected:
		void changeEvent (QEvent*) override;
		void resizeEvent (QResizeEvent*) override;
	private:
		void UpdateElide ();
	};
}
