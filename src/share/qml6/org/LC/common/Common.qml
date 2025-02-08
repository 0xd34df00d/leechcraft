import QtQuick

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

    property var tooltips: ({})

    function closeTooltip(path) {
        if (!tooltips[path])
            return false;

        const tooltip = tooltips[path];
        tooltip.close();
        tooltip.destroy();
        delete tooltips[path];
        return true;
    }

    function openTooltip(item, params, path) {
        if (tooltips[path])
            return;

        const component = Qt.createComponent(path);
        const global = getTooltipPos(item);

        const sf = () => {
            const tooltip = component.createObject(item, params);
            const { x, y } = quarkProxy.fitRect(Qt.point(global.x, global.y),
                    Qt.size(tooltip.width, tooltip.height),
                    quarkProxy.getWinRect(),
                    false);
            tooltip.x = x;
            tooltip.y = y;
            tooltip.show();
            quarkProxy.registerAutoresize(Qt.point(global.x, global.y), tooltip);
            tooltips[path] = tooltip;

            if (tooltip.closing) {
                tooltip.closing.connect(() => delete tooltips[path]);
            }
        };

        if (component.status === Component.Error)
            console.error("Error opening window for " + path + ": " + component.errorString());

        if (component.status === Component.Ready)
            sf();
        else
            component.onCompleted = sf;
    }

    function toggleTooltip(item, params, path) {
        closeTooltip(path) || openTooltip(item, params, path);
    }
}
