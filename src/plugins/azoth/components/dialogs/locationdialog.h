/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_AZOTH_LOCATIONDIALOG_H
#define PLUGINS_AZOTH_LOCATIONDIALOG_H
#include <QDialog>
#include "interfaces/azoth/isupportgeolocation.h"
#include "ui_locationdialog.h"

namespace LC
{
namespace Azoth
{
	class LocationDialog : public QDialog
	{
		Ui::LocationDialog Ui_;
	public:
		LocationDialog (QWidget* = 0);
		
		void SetInfo (const GeolocationInfo_t&);
		GeolocationInfo_t GetInfo () const;
	};
}
}

#endif
