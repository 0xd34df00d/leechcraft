import QtQuick 1.1

Rectangle
{
	id: recentCommentsRect
	anchors.fill: parent

	ListModel
	{
		id: commentsModel

		ListElement
		{
			subject: "subject"
			body: "body"
			date: "date"
			author: "author"
		}

		ListElement
		{
			subject: "subject1"
			body: "body1"
			date: "date1"
			author: "author1"
		}

		ListElement
		{
			subject: "subject2"
			body: "body2"
			date: "date2"
			author: "author2"
		}
	}

	ListView
	{
		id: recentCommentsView
		clip: true
		anchors.fill: parent
		model: commentsModel;
		highlight: Rectangle { color: "#DCDCDC"; }
		focus: true

		boundsBehavior: Flickable.StopAtBounds;
		delegate: Item {
			height: 80
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
				border.color: "black"
				smooth: true

				Text
				{
					id: subjectText
					text: subject
					font.bold: true
					font.underline: true
					font.pointSize: 12
					color: "black"
					anchors.top: parent.top
					anchors.topMargin: 2
					anchors.left: delegateRect.left
					anchors.leftMargin: 5
					anchors.right: delegateRect.right
					anchors.rightMargin: 5
				}

				Text {
					id: bodyText
					text: body
					color: "black"
					anchors.top: subjectText.bottom
					anchors.topMargin: 2
					anchors.left: delegateRect.left
					anchors.leftMargin: 5
					anchors.right: delegateRect.right
					anchors.rightMargin: 5
					elide: Text.ElideRight
					font.pointSize: 8
				}

				Text {
					id: dateText
					text: date
					color: "black"
					anchors.top: bodyText.bottom
					anchors.topMargin: 2
					anchors.left: delegateRect.left
					anchors.leftMargin: 5
					anchors.right: delegateRect.right
					anchors.rightMargin: 5
				}

				Text {
					id: authorText
					text: author
					color: "black"
					anchors.top: dateText.bottom
					anchors.topMargin: 2
					anchors.left: delegateRect.left
					anchors.leftMargin: 5
					anchors.right: delegateRect.right
					anchors.rightMargin: 5
				}
			}
		}
	}
}
