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

	signal linkActivated (string url)

	ListView
	{
		id: recentCommentsListView
		clip: true
		anchors.fill: parent
		model: recentCommentsModel;
		focus: true

		boundsBehavior: Flickable.StopAtBounds;
		delegate: Item
		{
			height: 100
			width: parent.width
			smooth: true

			effect: Blur
			{
				id: recentCommentsBlur
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

				border.width: 1
				border.color: colorProxy.color_TextBox_BorderColor
				smooth: true

				Text
				{
					id: subjectText

					text: nodeSubject

					font.bold: true
					font.underline: true
					font.pointSize: 12

					color: colorProxy.color_TextBox_TitleTextColor

					anchors.top: parent.top
					anchors.topMargin: 1
					anchors.left: delegateRect.left
					anchors.leftMargin: 5
					anchors.right: delegateRect.right
					anchors.rightMargin: 5
				}

				Text
				{
					id: commentText

					width: parent.width
					height: 50
					text: commentBody
					textFormat: Text.RichText
					wrapMode: Text.WordWrap
					elide: Text.ElideRight

					clip: true

					color: colorProxy.color_TextBox_TextColor

					anchors.top: subjectText.bottom
					anchors.topMargin: 1
					anchors.left: delegateRect.left
					anchors.leftMargin: 5
					anchors.right: delegateRect.right
					anchors.rightMargin: 5
				}

				Text
				{
					id: commentInfoText

					font.pointSize: 8

					text: commentInfo
					elide: Text.ElideRight

					color: colorProxy.color_TextBox_Aux1TextColor

					anchors.left: delegateRect.left
					anchors.leftMargin: 5
					anchors.right: delegateRect.right
					anchors.rightMargin: 5
					anchors.bottom: delegateRect.bottom
					anchors.bottomMargin: 1
				}

				MouseArea
				{
					id: subjectTextMouseArea
					anchors.fill: subjectText
					hoverEnabled: true
					z: 1

					onEntered:
						recentCommentsView.setItemCursor (subjectTextMouseArea, "PointingHandCursor");
					onExited:
						recentCommentsView.setItemCursor (subjectTextMouseArea, "ArrowCursor");
					onClicked:
						rootRect.linkActivated (nodeUrl)
				}

				MouseArea
				{
					id: rectMouseArea
					z: 0
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
}
