import QtQuick 2.0

QtObject {
    function getTooltipPos(item) {
        var absPoint = item.mapToItem(quarkDisplayRoot, 0, 0);
        absPoint = quarkProxy.mapToGlobal(absPoint.x, absPoint.y);
        return absPoint;
    }

    function showTooltip(item, func) {
        var absPoint = getTooltipPos(item);
        func(absPoint.x, absPoint.y);
    }

    function showTextTooltip(item, text) {
        var absPoint = getTooltipPos(item);
        absPoint.x += item.width;
        absPoint.y += item.height;
        quarkProxy.showTextTooltip(absPoint.x, absPoint.y, text);
    }
}
