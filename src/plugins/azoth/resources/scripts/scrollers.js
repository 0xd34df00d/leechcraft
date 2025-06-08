"use strict";

window.ShouldScroll = true;

const ScrollToBottom = (force) => {
    if (window.ShouldScroll || force)
        window.scrollTo(0, document.body.scrollHeight);
}

const TestScroll = () => {
    window.ShouldScroll = document.body.scrollHeight <= (window.innerHeight + window.scrollY + window.innerHeight / 5);
}

(() => {
    const scheduleScroll = () => { setTimeout(ScrollToBottom, 0); };

    const observer = new MutationObserver(scheduleScroll);
    observer.observe(document.body, {
        childList: true,
        subtree: true,
        characterData: true
    });

    window.addEventListener('resize', scheduleScroll, { passive: true });
    window.addEventListener('scroll', TestScroll, { passive: true });
})();

ScrollToBottom(true);

const ScrollPage = (direction) => {
    window.scrollBy({
        top: (window.innerHeight / 2 - 5) * direction,
        left: 0,
        behavior: 'smooth'
    });
}
