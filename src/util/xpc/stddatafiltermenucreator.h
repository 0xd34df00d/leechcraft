/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include "xpcconfig.h"

class QVariant;
class IEntityManager;
class QMenu;

namespace LC::Util
{
	/** @brief Adds actions to handle some data with relevant IDataFilter
	 * plugins to a menu.
	 *
	 * This class can be used to offer a list of possible variants of
	 * handling an piece of data (like selected text, image, and so on)
	 * to the user.
	 *
	 * For example, a hypothetical text editor could handle context menu
	 * requests in the following way:
	 * \code{.cpp}
		const auto iem = Proxy_->GetEntityManager ();		// Proxy_ is a ICoreProxy_ptr object
		QMenu menu;

		const auto& image = GetSelectionImage ();			// returns selected image part of a document as a QImage
		new Util::StdDataFilterMenuCreator { image, iem, &menu };

		const auto& text = GetSelectionText ();				// returns selected text part of a document as a QString
		new Util::StdDataFilterMenuCreator { text, iem, &menu };

		menu.exec ();
	   \endcode
	 *
	 * @sa IDataFilter
	 */
	class UTIL_XPC_API StdDataFilterMenuCreator : public QObject
	{
		IEntityManager * const EntityMgr_;

		QByteArray ChosenPlugin_;
		QByteArray ChosenVariant_;
	public:
		/** @brief Constructs the StdDataFilterMenuCreator.
		 *
		 * The newly created StdDataFilterMenuCreator becomes a child of
		 * the passed \em menu, so there is no need to delete it
		 * explicitly.
		 *
		 * The actions corresponding to the data filter plugins are
		 * appended to the end of the menu.
		 *
		 * @param[in] data The data to try to handle with IDataFilter
		 * plugins.
		 * @param[in] iem The IEntityManager object for obtaining the
		 * list of IDataFilter plugins.
		 * @param[in] menu The menu to add actions to.
		 */
		StdDataFilterMenuCreator (const QVariant& data, IEntityManager *iem, QMenu *menu);

		/** @brief Returns the ID of the chosen plugin.
		 *
		 * @return The ID of the chosen plugin, or an empty QByteArray if
		 * no plugin is chosen yet.
		 *
		 * @sa GetChosenVariant()
		 */
		const QByteArray& GetChosenPlugin () const;

		/** @brief Returns the ID of the chosen data filter variant.
		 *
		 * The ID corresponds to the ID of the IDataFilter::FilterVariant
		 * structure.
		 *
		 * @return The ID of the chosen data filter variant, or an empty
		 * QByteArray if no plugin is chosen yet.
		 *
		 * @sa GetChosenVariant()
		 * @sa IDataFilter
		 * @sa IDataFilter::FilterVariant
		 */
		const QByteArray& GetChosenVariant () const;
	};
}
