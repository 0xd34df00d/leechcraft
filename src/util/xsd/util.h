/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include "xsdconfig.h"

class QString;

namespace LC
{
namespace Util
{
	class XmlSettingsDialog;
	class BaseSettingsManager;

	/** @brief Opens XML settings dialog for the given XML \em filename.
	 *
	 * The dialog is opened as non-modal and non-blocking and is
	 * automatically shown. The dialog is also set to be automatically
	 * deleted as soon as it is closed.
	 *
	 * @param[in] title The title of the dialog.
	 * @param[in] filename The XML settings file to use to build the
	 * dialog.
	 * @param[in] bsm The instance of BaseSettingsManager to use for
	 * storing the settings.
	 * @return The XML settings dialog.
	 */
	UTIL_XSD_API XmlSettingsDialog* OpenXSD (const QString& title, const QString& filename, Util::BaseSettingsManager *bsm);
}
}
