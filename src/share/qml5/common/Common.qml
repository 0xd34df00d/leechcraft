import QtQuick 2.3

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

    function openWindow(item, params, path, old, setter) {
        if (old)
            old.destroy();

        var component = Qt.createComponent(path);
        var global = commonJS.getTooltipPos(item);

        var sf = (function() {
                var tooltip = component.createObject(item, params);
                var pos = quarkProxy.fitRect(Qt.point(global.x, global.y),
                        Qt.size(tooltip.width, tooltip.height),
                        quarkProxy.getWinRect(),
                        false);
                tooltip.x = pos.x;
                tooltip.y = pos.y;
                tooltip.show();
                setter(tooltip);
            });

        if (component.status === Component.Error)
            console.log("Error opening window for " + path + ": " + component.errorString());

        if (component.status === Component.Ready)
            sf();
        else
            component.onCompleted = sf;
    }
}
