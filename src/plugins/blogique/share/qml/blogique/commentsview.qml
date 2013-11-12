import QtQuick 1.1
import Effects 1.0

Rectangle
{
	id: rootRect

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
			height: 150
			width: parent.width
			smooth: true

			effect: Blur
			{
				blurRadius: 0.0
			}

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
					anchors.right: delegateRect.right
					anchors.rightMargin: 5
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

						onEntered:
							parentWidget.setItemCursor (subjectTextMouseArea,
									"PointingHandCursor");
						onExited:
							parentWidget.setItemCursor (subjectTextMouseArea,
									"ArrowCursor");
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
					anchors.right: delegateRect.right
					anchors.rightMargin: 5
				}

				Flickable
				{
					id: commentBodyFlickable
					contentWidth: parent.width - 10
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
					anchors.right: delegateRect.right
					anchors.rightMargin: 5

					interactive: commentBodyText.paintedHeight > commentBodyFlickable.height

					Text
					{
						id: commentBodyText
						width: parent.width

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
					text: commentAuthor

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
					}
				}
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

// 	signal linkActivated (string url)


//
//
// 				Text
// 				{
// 					id: commentInfoText
//
// 					font.pointSize: 8
//
// 					text: commentInfo
// 					elide: Text.ElideRight
//
// 					color: colorProxy.color_TextBox_Aux1TextColor
//
// 					anchors.left: delegateRect.left
// 					anchors.leftMargin: 5
// 					anchors.right: delegateRect.right
// 					anchors.rightMargin: 5
// 					anchors.bottom: delegateRect.bottom
// 					anchors.bottomMargin: 1
// 				}
//
// 				MouseArea
// 				{
// 					id: subjectTextMouseArea
// 					anchors.fill: subjectText
// 					hoverEnabled: true
// 					z: 1
//
// 					onEntered:
// 					recentCommentsView.setItemCursor (subjectTextMouseArea, "PointingHandCursor");
// 					onExited:
// 					recentCommentsView.setItemCursor (subjectTextMouseArea, "ArrowCursor");
// 					onClicked:
// 					rootRect.linkActivated (nodeUrl)
// 				}
//
// 				MouseArea
// 				{
// 					id: rectMouseArea
// 					z: 0
// 					anchors.fill: parent
// 					hoverEnabled: true
// 				}
//
// 				states:
// 				[
// 				State
// 				{
// 					name: "hovered"
// 					when: rectMouseArea.containsMouse
// 					PropertyChanges { target: delegateRect; border.color: colorProxy.color_TextBox_HighlightBorderColor }
// 					PropertyChanges { target: upperStop; color: colorProxy.color_TextBox_HighlightTopColor }
// 					PropertyChanges { target: lowerStop; color: colorProxy.color_TextBox_HighlightBottomColor }
// 				}
// 				]
//
// 				transitions:
// 				[
// 				Transition
// 				{
// 					from: ""
// 					to: "hovered"
// 					reversible: true
// 					PropertyAnimation { properties: "border.color"; duration: 200 }
// 					PropertyAnimation { properties: "color"; duration: 200 }
// 				}
// 				]
// 			}
// 		}
// 	}
}