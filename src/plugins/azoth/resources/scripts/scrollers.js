"use strict";

const InitializeAutoscroll = () => {
    const scrollingElement = document.documentElement;

    const doScroll = () => {
        const scrollTop = scrollingElement.scrollTop;
        const scrollHeight = scrollingElement.scrollHeight;
        const clientHeight = window.innerHeight;
        const distanceToBottom = scrollHeight - (scrollTop + clientHeight);

        if (distanceToBottom < clientHeight / 5) {
            window.scrollTo(0, scrollHeight);
        }
    };

    const observer = new MutationObserver(doScroll);
    observer.observe(scrollingElement, {
        childList: true,
        subtree: true,
        characterData: true
    });

    let resizeTimeout = null;
    window.addEventListener('resize', () => {
        if (resizeTimeout) {
            return;
        }
        doScroll();
        resizeTimeout = setTimeout(() => {
                doScroll();
                resizeTimeout = null;
            },
            100);
    });
};

InitializeAutoscroll();

const ScrollPage = (direction) => {
    window.scrollBy({
        top: (window.innerHeight / 2 - 5) * direction,
        left: 0,
        behavior: 'smooth'
    });
}
