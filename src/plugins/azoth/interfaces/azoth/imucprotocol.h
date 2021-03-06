/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QVariantMap>

class QWidget;

namespace LC
{
namespace Azoth
{
	/** @brief Interface for protocols supporting multiuser chat rooms.
	 *
	 * This interface should be implemented by protocol objects supporting
	 * multiuser chat rooms.
	 */
	class IMUCProtocol
	{
	public:
		virtual ~IMUCProtocol () {};

		/** @brief Returns the widget used to set up the MUC join options.
		 *
		 * The returned widget must implement IMUCJoinWidget.
		 *
		 * The caller takes the ownership of the widget, so each time
		 * a newly constructed widget should be returned, and the plugin
		 * shouldn't delete the widget by itself.
		 *
		 * If the protocol doesn't support joining multi-user chats, it is
		 * safe to return \b nullptr here.
		 *
		 * @return The widget used for joining MUCs, which must implement
		 * IMUCJoinWidget, or \b nullptr if not supported.
		 *
		 * @sa IMUCJoinWidget
		 */
		virtual QWidget* GetMUCJoinWidget () = 0;

		/** @brief Tries to guess MUC identifying data from user input.
		 *
		 * This method is used by Azoth core and other plugins to try to
		 * guess MUC identifying data from the given user \em input.
		 * The \em input, for example, may be a string like
		 * <em>c_plus_plus@conference.jabber.ru</em> in case of a XMPP
		 * MUC.
		 *
		 * The method should return a variant map suitable to passing
		 * to IMUCJoinWidget::SetIdentifyingData().
		 *
		 * The \em entryObj can be used to get additional information
		 * about the context of the user input. For example, if only room
		 * name is given, it can be used to get the corresponding server
		 * address.
		 *
		 * @param[in] input The user input, like a human-readable MUC
		 * name.
		 * @param[in] entryObj The entry this input is relevant to.
		 * @return The MUC identifying data suitable for passing to
		 * IMUCJoinWidget::SetIdentifyingData(), or an empty variantmap
		 * if guessing failed.
		 *
		 * @sa IMUCJoinWidget::SetIdentifyingData()
		 */
		virtual QVariantMap TryGuessMUCIdentifyingData (const QString& input, QObject *entryObj)
		{
			Q_UNUSED (input);
			Q_UNUSED (entryObj);
			return {};
		}
	};
}
}

Q_DECLARE_INTERFACE (LC::Azoth::IMUCProtocol,
		"org.Deviant.LeechCraft.Azoth.IMUCProtocol/1.0")
