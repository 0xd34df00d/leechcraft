import QtQuick 1.1

Rectangle
{
	id: recentCommentsWidget
	width: main.width
	height: main.height

	Rectangle
	{
		id: main
		x: 0
		y: 0

		Rectangle
		{
			id: recentCommentsRect
			anchors.fill: parent

			Component
			{
				id: recentCommentDelegate

				Item
				{
					id: wrapper;
				}
			}

			ListView
			{
				id: recentCommentsView
				clip: true
				anchors.fill: parent
//				model: serversModel.count ? serversModel : dummyModel;
//				delegate: serversModel.count ?  contactDelegate : dummyDelegate
				highlight: Rectangle { color: "#DCDCDC"; }
				focus: true

				boundsBehavior: Flickable.StopAtBounds;
			}
		}
	}
}
