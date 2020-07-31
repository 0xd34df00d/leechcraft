/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2013  Oleg Linkin <MaledictusDeMagog@gmail.com>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QTimer>
#include <QQuickItem>
#include "qmlconfig.h"

namespace LC
{
namespace Util
{
	/** @brief ToolTip for Qml objects.
	 *
	 * Rich text is supported.
	 *
	 * Using the tooltip is pretty easy.
	 * First of all register tooltip in your widget:
	 * \code{.cpp}
		qmlRegisterType<Util::ToolTipItem> ("org.LC.common", 1, 0, "ToolTip");
	   \endcode
	 *
	 * Then in yout qml import this widget:
	 * \code{.qml}
		import org.LC.common 1.0
	   \endcode
	 *
	 * And now you can use tooltip:
	 * \code{.qml}
		Rectangle {
			anchors.fill: parent

			MouseArea {
				anchors.fill: subjectText
				hoverEnabled: true
				ToolTip {
					anchors.fill: parent
					text: "tooltip text"
				}
			}
		}
	   \endcode
	 *
	 * @ingroup QmlUtil
	 */
	class UTIL_QML_API ToolTipItem : public QQuickItem
	{
		Q_OBJECT

		/** @brief The text of this tooltip item (rich text supported).
		 */
		Q_PROPERTY (QString text READ GetText WRITE SetText NOTIFY textChanged)

		/** @brief Whether this tooltip contains mouse.
		 */
		Q_PROPERTY (bool containsMouse READ ContainsMouse NOTIFY containsMouseChanged)

		QTimer ShowTimer_;
		QString Text_;
		bool ContainsMouse_ = false;
	public:
		/** @brief Constructs the tooltip with the given \em parent item.
		 *
		 * @param[in] parent The parent item for this tooltip.
		 */
		ToolTipItem (QQuickItem *parent = nullptr);

		/** @brief Sets the text contained in this tooltip to \em text.
		 *
		 * @param[in] text The text of this tooltip.
		 *
		 * @sa GetText()
		 */
		void SetText (const QString& text);

		/** @brief Returns the text of this tooltip.
		 *
		 * @return The text contained in this tooltip.
		 *
		 * @sa SetText()
		 */
		QString GetText () const;

		/** @brief Returns whether the tooltip contains the mouse.
		 *
		 * @return Whether the tooltip contains the mouse pointer.
		 */
		bool ContainsMouse () const;

		/** @brief Shows tooltip with the given text immediately.
		 *
		 * The passed \em text overrides the \em text property of this
		 * tooltip, but does not change it.
		 */
		void ShowToolTip (const QString& text) const;
	protected:
		void hoverEnterEvent (QHoverEvent*) override;
		void hoverLeaveEvent (QHoverEvent*) override;
	public slots:
		/** @brief Shows the tooltip immediately.
		 */
		void showToolTip ();
	signals:
		/** @brief Emitted when the text of this tooltip changes.
		 */
		void textChanged ();

		/** @brief Emitted when the containsMouse property changes.
		 */
		void containsMouseChanged ();
	};
}
}
