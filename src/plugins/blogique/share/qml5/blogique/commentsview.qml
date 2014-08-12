import QtQuick 2.3
import org.LC.common 1.0

Rectangle
{
	id: rootRect

	signal linkActivated (string url)
	signal deleteComment (string accountId, int commentId)
	signal markCommentAsRead (string accotunId, int commentId)
	signal addComment (string accountId, int entryID, int commentId)

	gradient: Gradient
	{
		GradientStop
		{
			position: 0
			color: colorProxy.color_TextView_TopColor
		}
		GradientStop
		{
			position: 1
			color: colorProxy.color_TextView_BottomColor
		}
	}

	anchors.fill: parent

	VisualDataModel
	{
		id: commentsVisualModel
		model: commentsModel

		delegate: Item
		{
			id: delegateRoot
			height: 150
			width: parent.width
			smooth: true

			Rectangle
			{
				id: delegateRect

				anchors.fill: parent
				anchors.leftMargin: 10
				anchors.rightMargin: 10
				anchors.topMargin: 5
				anchors.bottomMargin: 5

				radius: 3

				border.width: 1
				border.color: colorProxy.color_TextBox_BorderColor
				smooth: true

				gradient: Gradient
				{
					GradientStop
					{
						position: 0
						id: upperStop
						color: colorProxy.color_TextBox_TopColor
					}
					GradientStop
					{
						position: 1
						id: lowerStop
						color: colorProxy.color_TextBox_BottomColor
					}
				}

				Text
				{
					id: entrySubjectText

					anchors.top: parent.top
					anchors.topMargin: 1
					anchors.left: delegateRect.left
					anchors.leftMargin: 5
					anchors.right: commonActionsColumn.left
					anchors.rightMargin: 1
					color: colorProxy.color_TextBox_TitleTextColor

					text: entrySubject

					visible: text != ""

					elide: Text.ElideRight

					font.bold: true
					font.underline: subjectTextMouseArea.containsMouse
					font.pointSize: 12

					MouseArea
					{
						id: subjectTextMouseArea

						anchors.fill: entrySubjectText
						hoverEnabled: true

						ToolTip
						{
							anchors.fill: parent
							text: entrySubject
						}
                        cursorShape: Qt.PointingHandCursor

						onClicked: linkActivated (entryUrl)
					}
				}

				Text
				{
					id: commentSubjectText

					width: parent.width

					text: commentSubject
					elide: Text.ElideRight
					clip: true
					font.bold: true
					color: colorProxy.color_TextBox_TextColor

					visible: text != ""

					anchors.top: entrySubjectText.bottom
					anchors.topMargin: 1
					anchors.left: delegateRect.left
					anchors.leftMargin: 5
					anchors.right: commonActionsColumn.left
					anchors.rightMargin: 1
				}

				Flickable
				{
					id: commentBodyFlickable
					width: parent.width - 24
					contentWidth: width
					contentHeight: commentBodyText.paintedHeight
					clip: true

					anchors.top: commentSubjectText.text == "" ?
						entrySubjectText.bottom :
						commentSubjectText.bottom
					anchors.topMargin: 1
					anchors.bottom: dateText.top
					anchors.bottomMargin: 5
					anchors.left: delegateRect.left
					anchors.leftMargin: 5
					anchors.right: commonActionsColumn.left
					anchors.rightMargin: 1

					interactive: commentBodyText.paintedHeight > commentBodyFlickable.height

					Text
					{
						id: commentBodyText
						anchors.top: parent.top
						anchors.left: parent.left
						anchors.right: parent.right
						anchors.bottom: parent.bottom

						text: commentBody

						textFormat: Text.RichText
						wrapMode: Text.Wrap

						clip: true
						color: colorProxy.color_TextBox_TextColor
					}
				}

				Text
				{
					id: dateText
					text: commentDate

					width: delegateRect.width - authorNameText.width - 10

					anchors.left: parent.left
					anchors.leftMargin: 5
					anchors.bottom: delegateRect.bottom
					anchors.bottomMargin: 1

					elide: Text.ElideRight
					horizontalAlignment: Text.AlignLeft
					color: colorProxy.color_TextBox_Aux1TextColor
				}

				Text
				{
					id: authorNameText
					text: commentAuthor == "" ? "Anonymous" : commentAuthor

					font.underline: authorNameTextMouseArea.containsMouse

					anchors.right: parent.right
					anchors.rightMargin: 5
					anchors.bottom: delegateRect.bottom
					anchors.bottomMargin: 1

					horizontalAlignment: Text.AlignRight
					color: colorProxy.color_TextBox_Aux1TextColor

					MouseArea
					{
						id: authorNameTextMouseArea

						anchors.fill: authorNameText
						hoverEnabled: true

						onEntered:
							parentWidget.setItemCursor (authorNameTextMouseArea,
									"PointingHandCursor");
						onExited:
							parentWidget.setItemCursor (authorNameTextMouseArea,
									"ArrowCursor");
						onClicked:
							if (commentAuthor != "")
								linkActivated ("http://" + authorNameText.text + ".livejournal.com")
					}
				}

				property bool isHovered: rectMouseArea.containsMouse ||
						openInBrowserAction.isHovered ||
						markAsReadAction.isHovered ||
						deleteCommentAction.isHovered  ||
						addCommentAction.isHovered
				
				Column
				{
					id: commonActionsColumn

					anchors.top: parent.top
					anchors.right: parent.right
					anchors.bottom: authorNameText.top
					anchors.bottomMargin: 1

					ActionButton {
						id: openInBrowserAction

						width: 24
						height: width

						opacity: delegateRect.isHovered ? 1 : 0
						Behavior on opacity { PropertyAnimation {} }
						
						textTooltip: qsTr ("Open in browser")

						actionIconURL: "image://ThemeIcons/go-jump-locationbar"
						onTriggered: rootRect.linkActivated (commentUrl)
					}

					ActionButton {
						id: markAsReadAction

						width: 24
						height: width

						opacity: delegateRect.isHovered ? 1 : 0
						Behavior on opacity { PropertyAnimation {} }
						
						textTooltip: qsTr ("Mark as read")

						actionIconURL: "image://ThemeIcons/mail-mark-read"
						onTriggered: rootRect.markCommentAsRead (accountID, commentID)
					}

					ActionButton {
						id: addCommentAction

						width: 24
						height: width

						opacity: delegateRect.isHovered ? 1 : 0
						Behavior on opacity { PropertyAnimation {} }
						
						textTooltip: qsTr ("Add comment")

						actionIconURL: "image://ThemeIcons/list-add"
						onTriggered: rootRect.addComment (accountID, entryID, commentID)
					}
					
					ActionButton {
						id: deleteCommentAction

						width: 24
						height: width

						opacity: delegateRect.isHovered ? 1 : 0
						Behavior on opacity { PropertyAnimation {} }
						
						textTooltip: qsTr ("Delete comment")

						actionIconURL: "image://ThemeIcons/list-remove"
						onTriggered: rootRect.deleteComment (accountID, commentID)
					}
				}

				MouseArea
				{
					id: rectMouseArea
					z: -1
					anchors.fill: parent
					hoverEnabled: true
				}

				states:
				[
					State
					{
						name: "hovered"
						when: rectMouseArea.containsMouse
						PropertyChanges { target: delegateRect; border.color: colorProxy.color_TextBox_HighlightBorderColor }
						PropertyChanges { target: upperStop; color: colorProxy.color_TextBox_HighlightTopColor }
						PropertyChanges { target: lowerStop; color: colorProxy.color_TextBox_HighlightBottomColor }
					}
				]

				transitions:
				[
					Transition
					{
						from: ""
						to: "hovered"
						reversible: true
						PropertyAnimation { properties: "border.color"; duration: 200 }
						PropertyAnimation { properties: "color"; duration: 200 }
					}
				]
			}
		}
	}

	ListView
	{
		id: commentsView

		anchors.fill: parent
		clip: true
		boundsBehavior: Flickable.StopAtBounds

		model: commentsVisualModel
	}
}
