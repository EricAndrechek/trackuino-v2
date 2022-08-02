const options = {
    bottom: '32px', // default: '32px'
    right: '32px', // default: '32px'
    left: 'unset', // default: 'unset'
    time: '0s', // default: '0.3s'
    mixColor: '#fff', // default: '#fff'
    backgroundColor: '#fff', // default: '#fff'
    buttonColorDark: '#100f2c', // default: '#100f2c'
    buttonColorLight: '#fff', // default: '#fff'
    saveInCookies: true, // default: true,
    label: '🌓', // default: ''
    autoMatchOsTheme: true, // default: true
};
let darkmode;
let is_dark;

// run code after page has loaded
function ready(fn) {
    if (document.readyState != 'loading') {
        fn();
    } else {
        document.addEventListener('DOMContentLoaded', fn);
    }
}

// page load
ready(function () {
    // determine page darkmode
    darkmode = new Darkmode(options);
    darkmode.showWidget();

    // Options for the observer (which mutations to observe)
    const config = { attributes: true, childList: false, subtree: false };

    // Callback function to execute when mutations are observed
    const callback = function (mutationsList, observer) {
        for (const mutation of mutationsList) {
            // set min height when dom shifts
            setMinHeight();
            if (mutation.type === 'attributes') {
                if (is_dark !== darkmode.isActivated()) {
                    update_github_buttons();
                }
            }
        }
    };

    // Create an observer instance linked to the callback function
    const observer = new MutationObserver(callback);

    // Start observing the target node for configured mutations
    observer.observe(document.body, config);

    // set button colors
    update_github_buttons();

    // set min height
    setMinHeight();
});

window.addEventListener('resize', setMinHeight);

// github button style rendering:

const star_light = `
<a
    class="github-button button-underneath"
    href="https://github.com/EricAndrechek/Trackuino-v2"
    data-color-scheme="light"
    data-icon="octicon-star"
    data-size="large"
    data-show-count="true"
    aria-label="Star EricAndrechek/Trackuino-v2 on GitHub"
    >Star</a
>
`;

const star_dark = `
<a
    class="github-button button-underneath"
    href="https://github.com/EricAndrechek/Trackuino-v2"
    data-color-scheme="dark"
    data-icon="octicon-star"
    data-size="large"
    data-show-count="true"
    aria-label="Star EricAndrechek/Trackuino-v2 on GitHub"
    >Star</a
>
`;

const issue_light = `
<a
    class="github-button"
    href="https://github.com/EricAndrechek/Trackuino-v2/issues"
    data-color-scheme="light"
    data-icon="octicon-issue-opened"
    data-size="large"
    data-show-count="true"
    aria-label="Issue EricAndrechek/Trackuino-v2 on GitHub"
    >Issue</a
>
`;

const issue_dark = `
<a
    class="github-button"
    href="https://github.com/EricAndrechek/Trackuino-v2/issues"
    data-color-scheme="dark"
    data-icon="octicon-issue-opened"
    data-size="large"
    data-show-count="true"
    aria-label="Issue EricAndrechek/Trackuino-v2 on GitHub"
    >Issue</a
>
`;

const sponsor_light = `
<a
    class="github-button"
    href="https://github.com/sponsors/EricAndrechek"
    data-color-scheme="light"
    data-icon="octicon-heart"
    data-size="large"
    aria-label="Sponsor @EricAndrechek on GitHub"
    >Sponsor</a
>
`;

const sponsor_dark = `
<a
    class="github-button"
    href="https://github.com/sponsors/EricAndrechek"
    data-color-scheme="dark"
    data-icon="octicon-heart"
    data-size="large"
    aria-label="Sponsor @EricAndrechek on GitHub"
    >Sponsor</a
>
`;

// hacky way to re-run the github button script when the theme changes
function update_github_buttons() {
    let btn_star = document.getElementById('btn-star');
    let btn_issue = document.getElementById('btn-issue');
    let btn_sponsor = document.getElementById('btn-sponsor');
    let buttons = [btn_star, btn_issue, btn_sponsor];
    delete_button_div_contents(buttons);

    is_dark = darkmode.isActivated();
    if (is_dark) {
        set_dark_buttons();
    } else {
        set_light_buttons();
    }

    let unique = Date.now();
    let scriptFile = document.createElement('script');
    scriptFile.type = 'text/javascript';
    scriptFile.async = true;
    scriptFile.src = 'https://buttons.github.io/buttons.js?unique=' + unique;
    let s = document.getElementsByTagName('script')[0];
    s.parentNode.insertBefore(scriptFile, s);
}

function delete_button_div_contents(buttons) {
    for (let i = 0; i < buttons.length; i++) {
        let button = buttons[i];
        button.innerHTML = '';
    }
}

function set_dark_buttons() {
    document.getElementById('btn-star').innerHTML = star_dark;
    document.getElementById('btn-issue').innerHTML = issue_dark;
    document.getElementById('btn-sponsor').innerHTML = sponsor_dark;
}

function set_light_buttons() {
    document.getElementById('btn-star').innerHTML = star_light;
    document.getElementById('btn-issue').innerHTML = issue_light;
    document.getElementById('btn-sponsor').innerHTML = sponsor_light;
}

function setMinHeight() {
    let screenHeight = window.innerHeight;

    let footerHeight = parseFloat(
        getComputedStyle(
            document.getElementsByClassName('footer')[0],
            null
        ).height.replace('px', '')
    );

    let minHeight = screenHeight - (footerHeight);

    document.getElementsByClassName("content-container")[0].style.minHeight =
        '' + minHeight + 'px';

    return minHeight;
}
