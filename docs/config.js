// run code after page has loaded
function ready(fn) {
    if (document.readyState != 'loading') {
        fn();
    } else {
        document.addEventListener('DOMContentLoaded', fn);
    }
}

ready(function () {
    uploadListener();
    configButtonSetup();
    fetch(
        'https://raw.githubusercontent.com/EricAndrechek/trackuino-v2/main/software/config.h?rnd=' +
            Date.now()
    )
        .then((response) => response.text())
        .then((text) => {
            parseConfig(text, true);
            raw_config = text;
            verifiedSchema = parsedConfigBuffer;
        });
});

const debug = false;

let parsedConfigBuffer;
let verifiedSchema;
let raw_config;

const uploadListener = () => {
    const fileInput = document.querySelector('#upload input[type=file]');
    fileInput.onchange = () => {
        if (fileInput.files.length > 0) {
            const fileName = document.querySelector('#upload .file-name');
            fileName.textContent = fileInput.files[0].name;
            const fr = new FileReader();
            fr.onload = () => {
                if (parseConfig(fr.result, false)) {
                    document
                        .getElementById('use-config')
                        .classList.remove('hidden');
                    document
                        .getElementById('upload-parse-error')
                        .classList.add('hidden');
                } else {
                    document
                        .getElementById('use-config')
                        .classList.add('hidden');
                    document
                        .getElementById('upload-parse-error')
                        .classList.remove('hidden');
                }
            };
            fr.readAsText(fileInput.files[0]);
        }
    };
};

const saveConfig = () => {
    // get the id and value of every input in the config-options id element
    const configOptions = document.getElementById('config-options');
    const fields = configOptions.getElementsByClassName('field');
    const config = {};
    for (let i = 0; i < fields.length; i++) {
        const field = fields[i];
        const input = field.getElementsByTagName('input')[0];
        const comment = field.getElementsByTagName('p')[0].innerHTML;
        const configKey = input.id.split('{')[1].split('}')[0];
        const configSubKey = input.id.split('{')[2].split('}')[0];
        const configSubKeyOption = input.id.split('{')[3].split('}')[0];
        let configValue;
        if (input.type === 'checkbox') {
            configValue = input.checked;
        } else {
            configValue = input.value;
        }
        if (config[configKey] === undefined) {
            config[configKey] = {};
        }
        if (config[configKey][configSubKey] === undefined) {
            config[configKey][configSubKey] = {};
        }
        config[configKey][configSubKey][configSubKeyOption] = {
            value: configValue,
            comment: comment,
        };
    }

    let configBuffer = `#ifndef __CONFIG_H__
#define __CONFIG_H__

// THIS IS THE TRACKUINO FIRMWARE CONFIGURATION FILE. YOUR CALLSIGN AND
// OTHER SETTINGS GO HERE.
//
// NOTE: all pins are Arduino based, not the Atmega chip. Mapping:
// https://www.arduino.cc/en/Hacking/PinMapping\n\n\n`;
    for (const key in config) {
        configBuffer +=
            '// --------------------------------------------------------------------------\n';
        configBuffer += '// ' + key + '\n';
        configBuffer +=
            '// --------------------------------------------------------------------------\n\n';
        for (const subKey in config[key]) {
            configBuffer += '// ' + subKey + '\n\n';
            for (const subKeyOption in config[key][subKey]) {
                let val = config[key][subKey][subKeyOption].value;
                const comment = config[key][subKey][subKeyOption].comment;
                if (val === true) {
                    configBuffer += '#define ' + subKeyOption + '\n';
                } else if (val === false) {
                    configBuffer += '// #define ' + subKeyOption + '\n';
                } else {
                    // TODO: build type checking logic in to verify type matching
                    if (isNaN(parseInt(val))) {
                        val = '"' + val + '"';
                    }
                    configBuffer +=
                        '#define ' + subKeyOption + ' ' + val + '\n';
                }
            }
            configBuffer += '\n';
        }
        configBuffer += '\n';
    }
    configBuffer += '\n#endif';

    document.getElementById('config-textarea').value = configBuffer;
    document.getElementById('config-options').classList.add('hidden');
    document.getElementById('save-config').classList.remove('hidden');
};

// from https://stackoverflow.com/a/30810322/7974356
function fallbackCopyTextToClipboard(text) {
    var textArea = document.createElement('textarea');
    textArea.value = text;

    // Avoid scrolling to bottom
    textArea.style.top = '0';
    textArea.style.left = '0';
    textArea.style.position = 'fixed';

    document.body.appendChild(textArea);
    textArea.focus();
    textArea.select();

    try {
        var successful = document.execCommand('copy');
        var msg = successful ? 'successful' : 'unsuccessful';
        console.log('Fallback: Copying text command was ' + msg);
    } catch (err) {
        console.error('Fallback: Oops, unable to copy', err);
    }

    document.body.removeChild(textArea);
}
function copyTextToClipboard(text) {
    if (!navigator.clipboard) {
        fallbackCopyTextToClipboard(text);
        return;
    }
    navigator.clipboard.writeText(text).then(
        function () {
            console.log('Async: Copying to clipboard was successful!');
        },
        function (err) {
            console.error('Async: Could not copy text: ', err);
        }
    );
}

const copyConfig = () => {
    document.getElementById('config-textarea').focus();
    document.getElementById('config-textarea').select();
    copyTextToClipboard(document.getElementById('config-textarea').value);
    document.getElementById('copy-success').classList.remove('hidden');
    setTimeout(() => {
        document.getElementById('copy-success').classList.add('hidden');
    }, 2000);
};

const downloadConfig = () => {
    const configText = document.getElementById('config-textarea').value;
    const blob = new Blob([configText], { type: 'text/plain' });
    const url = URL.createObjectURL(blob);
    const link = document.createElement('a');
    link.download = 'config.h';
    link.href = url;
    link.click();
    URL.revokeObjectURL(url);
};

const toggleSiblings = (elem) => {
    let sibling = elem.parentNode.firstChild;

    while (sibling) {
        if (sibling.nodeType === 1 && sibling !== elem) {
            sibling.classList.toggle('hidden');
        }
        sibling = sibling.nextSibling;
    }
};

const configSetup = () => {
    document.getElementById('upload').classList.add('hidden');
    const hr = document.createElement('hr');
    document.getElementById('config-options').appendChild(hr);
    // make input boxes or checkboxes for each config option
    for (const config in parsedConfigBuffer) {
        // make new div for config option
        const configDiv = document.createElement('div');
        configDiv.classList.add('config-option');
        // make new label for config option
        const configLabel = document
            .getElementById('section-dropdown-template')
            .cloneNode(true);
        configLabel.removeAttribute('id');
        configLabel.getElementsByTagName('span')[0].innerHTML = config;
        configLabel
            .getElementsByTagName('span')[0]
            .classList.add('config-label', 'title', 'is-4');
        configLabel.getElementsByTagName('span')[0].removeAttribute('id');
        configDiv.appendChild(configLabel);
        for (const subConfig in parsedConfigBuffer[config]) {
            // make new div for sub config option
            const subConfigDiv = document.createElement('div');
            subConfigDiv.classList.add('sub-config-option');
            // make new label for sub config option
            const subConfigLabel = document.createElement('p');
            subConfigLabel.classList.add(
                'sub-config-label',
                'subtitle',
                'is-4'
            );
            subConfigLabel.innerHTML = subConfig;
            subConfigDiv.appendChild(subConfigLabel);
            if (config.includes(' MODULE')) {
                subConfigDiv.classList.add('hidden');
            }
            for (const key in parsedConfigBuffer[config][subConfig]) {
                // make new div for config option
                const keyDiv = document.createElement('div');
                keyDiv.classList.add('key-option', 'field');
                // make new label for config option
                const keyLabel = document.createElement('label');
                keyLabel.innerHTML = key;
                keyLabel.classList.add('label');
                const helpLabel = document.createElement('p');
                helpLabel.innerHTML =
                    parsedConfigBuffer[config][subConfig][key]['comment'];
                helpLabel.classList.add('help');

                const keyControl = document.createElement('div');
                keyControl.classList.add('control');
                // make new input for config option
                const keyInput = document.createElement('input');
                if (
                    parsedConfigBuffer[config][subConfig][key]['value'] === true
                ) {
                    keyInput.setAttribute('type', 'checkbox');
                    keyInput.checked = true;
                    keyInput.classList.add('checkbox');
                } else if (
                    parsedConfigBuffer[config][subConfig][key]['value'] ===
                    false
                ) {
                    keyInput.setAttribute('type', 'checkbox');
                    keyInput.checked = false;
                    keyInput.classList.add('checkbox');
                } else {
                    keyInput.setAttribute('type', 'text');
                    keyInput.value =
                        parsedConfigBuffer[config][subConfig][key]['value'];
                    keyInput.classList.add('input');
                }
                keyInput.id =
                    '{' + config + '}{' + subConfig + '}{' + key + '}';
                keyControl.appendChild(keyInput);
                keyDiv.appendChild(keyLabel);
                keyDiv.appendChild(keyControl);
                keyDiv.appendChild(helpLabel);
                subConfigDiv.appendChild(keyDiv);
            }
            configDiv.appendChild(subConfigDiv);
        }
        document.getElementById('config-options').appendChild(configDiv);
    }
    const saveConfigButton = document.createElement('button');
    saveConfigButton.classList.add('button', 'is-link');
    saveConfigButton.innerHTML = 'Save Config';
    saveConfigButton.addEventListener('click', () => {
        saveConfig();
    });
    document.getElementById('config-options').appendChild(saveConfigButton);
    document.getElementById('config-options').classList.remove('hidden');
};

const configButtonSetup = () => {
    const newConfigButton = document.getElementById('new-config');
    const useConfigButton = document.getElementById('use-config');

    newConfigButton.addEventListener('click', () => {
        // fetch config from github
        fetch(
            'https://raw.githubusercontent.com/EricAndrechek/trackuino-v2/main/software/config.h?rnd=' +
                Date.now()
        )
            .then((response) => response.text())
            .then((text) => {
                if (parseConfig(text, false)) {
                    configSetup();
                } else {
                    console.log('new config parse error');
                }
            });
    });

    useConfigButton.addEventListener('click', () => {
        configSetup();
    });
};

const parseConfig = (configText, isSchema) => {
    // parse config text
    // verify that parsed config schema matches github config schema
    // return true if config valid
    // return false if config is invalid

    let stagingBuffer = {
        'GENERAL CONFIGURATION': {
            'Sensors Config': {},
        },
        'MODULE CONFIGURATION': {
            'Module Config': {},
        },
        'APRS MODULE CONFIGURATION': {
            'APRS Config': {},
            'AX.25 Config': {},
            'Tracker Config': {},
            'Modem Config': {},
            'Radio Config': {},
            'Debug Config': {},
        },
        'BUZZER MODULE CONFIGURATION': {
            'Buzzer Config': {},
        },
        'GPS MODULE CONFIGURATION': {
            'GPS Config': {},
        },
        'GSM MODULE CONFIGURATION': {
            'GSM Config': {},
        },
        'SD MODULE CONFIGURATION': {
            'SD Config': {},
        },
        'DEVELOPMENT CONFIGURATION': {
            'Power Config': {},
            'Tracker Config': {},
        },
    };

    let lastConfig = false;
    let lastSubConfig = false;

    try {
        configText = configText.split('\n');
        for (let i = 0; i < configText.length; i++) {
            let line = configText[i];
            if (line.startsWith('#define') && lastConfig && lastSubConfig) {
                // this is a value we need to parse
                let lineSplit = line.split(/\s+/);
                let key = lineSplit[1];
                let value =
                    lineSplit
                        .slice(2)
                        .join(' ')
                        .replace(/["']/g, '')
                        .split('//')[0] || true; // if no value, set to true
                if (value instanceof String && value.startsWith('//'))
                    value = true;
                if (debug) console.log(key, value);
                stagingBuffer[lastConfig][lastSubConfig][key] = {
                    value: value,
                    comment: '',
                };
            } else if (line.startsWith('/')) {
                line = line.substring(2);
                if (line.startsWith(' ')) line = line.substring(1);
                // check if line is a config header
                if (line in stagingBuffer) {
                    lastConfig = line;
                    if (debug) console.log(`found config: ${lastConfig}`);
                } else {
                    // remove parenthesis from line if there are any and then remove final whitespace character if there is one
                    line = line.split('(')[0].trim();
                    // check if line is a sub config header
                    if (lastConfig && line in stagingBuffer[lastConfig]) {
                        lastSubConfig = line;
                        if (debug)
                            console.log(`found sub config: ${lastSubConfig}`);
                    } else if (line.startsWith('#define')) {
                        // this is a boolean value that is disabled
                        let lineSplit = line.split(/\s+/);
                        let key = lineSplit[1];
                        let value =
                            lineSplit
                                .slice(2)
                                .join(' ')
                                .replace(/["']/g, '')
                                .split('//')[0] || undefined; // if no value, set to false
                        if (value === undefined || value.startsWith('//')) {
                            // we only want to mark it as a boolean variable if it is not just a commented out parameter
                            if (debug)
                                console.log(`found disabled config: ${key}`);
                            stagingBuffer[lastConfig][lastSubConfig][key] = {
                                value: false,
                                comment: '',
                            };
                        }
                    }
                }
            }
        }
        if (isSchema) {
            parsedConfigBuffer = stagingBuffer;
            return true;
        } else {
            // check that all keys in verifiedSchema are in stagingBuffer
            for (let config in verifiedSchema) {
                for (let subConfig in verifiedSchema[config]) {
                    for (let key in verifiedSchema[config][subConfig]) {
                        if (!(key in stagingBuffer[config][subConfig])) {
                            return false;
                        }
                    }
                }
            }
            parsedConfigBuffer = stagingBuffer;
            return true;
        }
    } catch (e) {
        if (debug) console.log(e);
        return false;
    }
};
