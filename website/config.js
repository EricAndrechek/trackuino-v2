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
        'https://raw.githubusercontent.com/EricAndrechek/trackuino-v2/main/software/config.h'
    )
        .then((response) => response.text())
        .then((text) => {
            parseConfig(text, true);
            verifiedSchema = parsedConfigBuffer;
        });
});

const debug = true;

let parsedConfigBuffer;
let verifiedSchema;

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
                        .classList.remove('is-hidden');
                    document
                        .getElementById('upload-parse-error')
                        .classList.add('is-hidden');
                } else {
                    document
                        .getElementById('use-config')
                        .classList.add('is-hidden');
                    document
                        .getElementById('upload-parse-error')
                        .classList.remove('is-hidden');
                }
            };
            fr.readAsText(fileInput.files[0]);
        }
    };
};

const configSetup = () => {
    document.getElementById('upload').classList.add('is-hidden');
    // need to now
};

const configButtonSetup = () => {
    const newConfigButton = document.getElementById('new-config');
    const useConfigButton = document.getElementById('use-config');

    newConfigButton.addEventListener('click', () => {
        // fetch config from github
        fetch(
            'https://raw.githubusercontent.com/EricAndrechek/trackuino-v2/main/software/config.h'
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
                let lineSplit = line.split(' ');
                let key = lineSplit[1];
                let value = lineSplit[2] || true; // if no value, set to true
                if (debug) console.log(key, value);
                stagingBuffer[lastConfig][lastSubConfig][key] = value;
            } else if (line.startsWith('/')) {
                line = line.substring(3);
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
                        let lineSplit = line.split(' ');
                        let key = lineSplit[1];
                        if (debug) console.log(`found disabled config: ${key}`);
                        stagingBuffer[lastConfig][lastSubConfig][key] = false;
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
