const versionNumber = "Version: 1.0.5";

// boolean to tell us if we've been offline and need to re-request old track data
let needData = true;

// position storage object
let positions = {};
// format:
// key = name
// value = {
/*
    name: name,
    symbol: symbol,
    last_update: datetime,
    current: {
        latitude: latitude,
        longitude: longitude,
        altitude: altitude,
        speed: speed,
        course: course,
        comment: comment,
        datetime: datetime,
    },
    previous_coordinates: [
        {
            latitude: latitude,
            longitude: longitude,
            altitude: altitude,
            speed: speed,
            course: course,
            comment: comment,
            datetime: datetime,
        },
        ...
    ],
*/

// telemetry storage object
let telemetry = {};

// prediction storage object
let predictions = {};

const debugLogs = () => {
    if (window.matchMedia("(orientation: portrait)").matches) {
        // open rickroll in new tab
        window.open("https://www.youtube.com/watch?v=dQw4w9WgXcQ", "_blank");
    } else if (window.matchMedia("(orientation: landscape)").matches) {
        console.log("Debug Logs");
        console.log("positions: ", positions);
        console.log("telemetry: ", telemetry);
        console.log("predictions: ", predictions);
    } else {
        console.log("Unknown orientation");
    }
};

const settings = () => {
    // get the settings from the settings form
    console.log("Settings");
    // show popup-page
    // check if popup-page is hidden
    if (document.getElementById("popup-page").classList.contains("page-show")) {
        // hide popup-page
        document.getElementById("popup-page").classList.remove("page-show");
    } else {
        // wipe contents of popup-page
        document.getElementById("popup-page").innerHTML = "";

        // populate settings form with current settings
        // fill with some unit selection options

        // create input elements
        const speed_units = document.createElement("select");
        speed_units.setAttribute("id", "speed-units");
        speed_units.setAttribute("name", "speed-units");
        speed_units.setAttribute("class", "input-select");
        const speed_units_options = ["m/s", "km/h", "mph", "knots"];
        for (let i = 0; i < speed_units_options.length; i++) {
            const option = document.createElement("option");
            option.setAttribute("value", speed_units_options[i]);
            option.innerText = speed_units_options[i];
            speed_units.appendChild(option);
        }
        // get speed_units from localStorage
        const speed_units_value = localStorage.getItem("speed_units");
        if (speed_units_value !== null) {
            speed_units.value = speed_units_value;
        }
        // create label for speed_units
        const speed_units_label = document.createElement("label");
        speed_units_label.setAttribute("for", "speed-units");
        speed_units_label.innerText = "Speed Units";

        const altitude_units = document.createElement("select");
        altitude_units.setAttribute("id", "altitude-units");
        altitude_units.setAttribute("name", "altitude-units");
        altitude_units.setAttribute("class", "input-select");
        const altitude_units_options = ["m", "ft"];
        for (let i = 0; i < altitude_units_options.length; i++) {
            const option = document.createElement("option");
            option.setAttribute("value", altitude_units_options[i]);
            option.innerText = altitude_units_options[i];
            altitude_units.appendChild(option);
        }
        // get altitude_units from localStorage
        const altitude_units_value = localStorage.getItem("altitude_units");
        if (altitude_units_value !== null) {
            altitude_units.value = altitude_units_value;
        }
        // create label for altitude_units
        const altitude_units_label = document.createElement("label");
        altitude_units_label.setAttribute("for", "altitude-units");
        altitude_units_label.innerText = "Altitude Units";

        // create p element for version number
        const version = document.createElement("p");
        // TODO: get version number from service-worker.js
        version.innerText = versionNumber;

        // create button for debug logs
        const debug = `<div id="debug-button" onclick="debugLogs()">Debug Logs</div>`;

        // create settings form
        const settings_form = document.createElement("form");
        settings_form.setAttribute("id", "settings-form");
        settings_form.setAttribute("class", "form");
        settings_form.appendChild(speed_units_label);
        settings_form.appendChild(speed_units);
        settings_form.appendChild(altitude_units_label);
        settings_form.appendChild(altitude_units);
        settings_form.appendChild(version);
        settings_form.innerHTML += debug;

        // onchange event listener for speed_units
        speed_units.addEventListener("change", (event) => {
            localStorage.setItem("speed_units", event.target.value);
        });

        // onchange event listener for altitude_units
        altitude_units.addEventListener("change", (event) => {
            localStorage.setItem("altitude_units", event.target.value);
        });

        // append settings form to popup-page
        document.getElementById("popup-page").appendChild(settings_form);

        const close = `<img id="close" onclick="closePopupPage()" src="/assets/x-mark.svg" alt="Close">`;

        // add close button to popup-page
        document.getElementById("popup-page").innerHTML += close;

        // show popup-page
        document.getElementById("popup-page").classList.add("page-show");
    }
};

const speedConversion = (speed) => {
    // convert speed to the units specified in localStorage
    let speed_units = localStorage.getItem("speed_units");
    let results = "";
    if (speed_units === "m/s") {
        results += (speed * 0.44704).toFixed(2) + " m/s";
    } else if (speed_units === "km/h") {
        results += (speed * 1.609344).toFixed(2) + " km/h";
    } else if (speed_units === "mph") {
        results += speed.toFixed(2) + " mph";
    } else if (speed_units === "knots") {
        results += (speed * 0.868976).toFixed(2) + " knots";
    } else {
        result += speed.toFixed(2) + " mph";
    }
    return results;
};

const altitudeConversion = (altitude) => {
    // convert altitude to the units specified in localStorage
    let altitude_units = localStorage.getItem("altitude_units");
    let results = "";
    if (altitude_units === "m") {
        results += altitude.toFixed(2) + " m";
    } else if (altitude_units === "ft") {
        results += (altitude * 3.28084).toFixed(2) + " ft";
    } else {
        results += altitude.toFixed(2) + " m";
    }
    return results;
};

const closePopupPage = () => {
    // check if popup-page is hidden
    if (document.getElementById("popup-page").classList.contains("page-show")) {
        // hide popup-page
        document.getElementById("popup-page").classList.remove("page-show");
    }
};

const getOrder = (name) => {
    // given a name as callsign - ssid, return an integer value for sorting
    let order = 0;
    // order cannot have decimals
    // name is callsign-ssid with -ssid being optional
    let callsign = name.split("-")[0];
    let ssid = name.split("-")[1];
    // callsign is 0-6 characters
    if (callsign.length > 6) {
        callsign = callsign.substring(0, 6);
    }
    // ssid is 0-2 characters
    if (ssid === undefined) {
        ssid = "0";
    } else if (ssid.length > 2) {
        ssid = ssid.substring(0, 2);
    }
    // callsign is 0-6 characters, ssid is 0-2 characters
    // must be less than max int value, but still unique
    // 0-6 characters = 0-6^26
    // 0-2 characters = 0-2^10
    // 6^26 = 308915776, 2^10 = 1024
    // 308915776 * 1024 = 316669952

    // callsign
    for (let i = 0; i < callsign.length; i++) {
        order += callsign.charCodeAt(i) * Math.pow(6, i);
    }
    // ssid
    for (let i = 0; i < ssid.length; i++) {
        order += ssid.charCodeAt(i) * Math.pow(2, i);
    }

    // not alphabetical, but unique I think?
    // could be very wrong, but limited testing shows works for now

    return order;
};

const nameToColor = (name) => {
    // given a name, return a unique color
    let hash = 0;
    for (let i = 0; i < name.length; i++) {
        hash = name.charCodeAt(i) + ((hash << 5) - hash);
    }
    let color = "#";
    color += ((hash >> 24) & 0xff).toString(16).padStart(2, "0");
    color += ((hash >> 16) & 0xff).toString(16).padStart(2, "0");
    color += ((hash >> 8) & 0xff).toString(16).padStart(2, "0");
    return color;
};

const getHistoricalGeoJSON = (name) => {
    let coordinates = [];
    if (name in positions) {
        for (let i = 0; i < positions[name].previous_coordinates.length; i++) {
            coordinates.push([
                positions[name].previous_coordinates[i].longitude,
                positions[name].previous_coordinates[i].latitude,
                positions[name].previous_coordinates[i].altitude,
                positions[name].previous_coordinates[i].speed,
                positions[name].previous_coordinates[i].course,
                positions[name].previous_coordinates[i].comment,
                positions[name].previous_coordinates[i].datetime,
            ]);
        }
    }
    const geojson = {
        type: "Feature",
        properties: {
            name: name,
            symbol: positions[name].symbol,
            last_update: positions[name].last_update,
        },
        geometry: {
            type: "LineString",
            coordinates: coordinates,
        },
    };
    // console.log(JSON.stringify(positions[name]));
    return geojson;
};

// display a message as a banner notification for timeout milliseconds
const notificationBanner = (message, timeout = 5000) => {
    let notification = document.getElementById("notification");
    notification.innerHTML = message;
    notification.classList.add("show");
    setTimeout(() => {
        notification.classList.remove("show");
    }, timeout);
};

const requestOldData = (names = null, age = 180) => {
    // fetch api.umich-balloons.com/api/getTail?name=NAME&age=AGE
    let url = `https://api.umich-balloons.com/api/getTail?name=${names}&age=${age}`;
    if (names === null) {
        url = `https://api.umich-balloons.com/api/getTail?age=${age}`;
    }
    fetch(url)
        .then((response) => {
            if (response.ok) {
                return response.json();
            } else {
                throw new Error("Failed to fetch data");
            }
        })
        .then((data) => {
            // for each name in data
            for (let name in data) {
                // check if name exists in positions
                if (!(name in positions)) {
                    positions[name] = {
                        name: name,
                        symbol: parseAPRSSymbol(data[name].symbol),
                        last_update: new Date(
                            Date.parse(data[name].last_updated + "Z")
                        ).toLocaleString(),
                        current: {
                            latitude: 0,
                            longitude: 0,
                            altitude: 0,
                            speed: 0,
                            course: 0,
                            comment: "",
                            datetime: "",
                        },
                        previous_coordinates: [],
                    };
                }
                // for each position in data[name]
                for (let i = data[name]["positions"].length - 1; i >= 0; i--) {
                    // add previous coordinates to previous_coordinates
                    let datatime;
                    try {
                        datetime = new Date(
                            Date.parse(data[name]["positions"][i].dt)
                        ).toLocaleString();
                    } catch (err) {
                        try {
                            datetime = new Date(
                                Date.parse(data[name]["positions"][i].dt)
                            ).toLocaleString();
                        } catch (err) {
                            console.log("Error parsing datetime: ", err);
                            datetime = new Date().toLocaleString();
                        }
                    }
                    // check if lat and lon are not both 0
                    // sorry people that need that specific coordinate at exactly 0.000000, 0.000000 :(
                    if (
                        data[name]["positions"][i].lat === 0 &&
                        data[name]["positions"][i].lon === 0
                    ) {
                        continue;
                    }
                    positions[name].previous_coordinates.push({
                        latitude: data[name]["positions"][i].lat,
                        longitude: data[name]["positions"][i].lon,
                        altitude: data[name]["positions"][i].alt,
                        speed: data[name]["positions"][i].spd,
                        course: data[name]["positions"][i].cse,
                        comment: data[name]["positions"][i].cmnt,
                        datetime: datetime,
                    });
                }
            }
            needData = false;
        })
        .catch((error) => {
            console.error("Error:", error);
            needData = true;
            notificationBanner("Failed to fetch old data", 10000);
        });
};

// main function
const main = () => {
    // check if has localStorage
    if (typeof Storage !== "undefined") {
        // if "has launched" is not in localStorage
        if (localStorage.getItem("hasLaunched") === null) {
            // create div element for popup-page contents
            const welcome = document.createElement("div");
            welcome.setAttribute("id", "welcome");

            // create title element
            const title = document.createElement("h1");
            title.innerText = "Welcome to umich-balloons!";

            // create p element for description
            const description = document.createElement("p");
            description.innerText =
                "This is a balloon tracking application made by Eric @ the University of Michigan. This application is designed to track the position of high altitude balloons in real-time. You can view the current position of the balloons, as well as their historical positions for up to 3 hours in the past. You can also view telemetry data for the balloons by clicking on the balloon. You can change the settings for the application by clicking the settings button in the top right corner.";

            const install = document.createElement("p");
            install.innerText =
                "For best performance when offline and while tracking multiple balloons, please install this web application on your device or add it to your home screen.";

            // create button element for close
            const close = `<img id="close" onclick="closePopupPage()" src="/assets/x-mark.svg" alt="Close">`;

            // create button element for dismissing message
            const button = `
                <button id="dismiss" onclick="localStorage.setItem('hasLaunched', 'true'); closePopupPage();">Dismiss</button>`;

            // append elements to welcome
            welcome.appendChild(title);
            welcome.appendChild(description);
            welcome.appendChild(install);
            welcome.innerHTML += button;
            welcome.innerHTML += close;

            // add to popup-page
            const popupPage = document.getElementById("popup-page");
            // clear popup-page
            popupPage.innerHTML = "";
            // add welcome to popup-page
            popupPage.appendChild(welcome);
            // show popup-page
            popupPage.classList.add("page-show");
        }
    } else {
        console.log("No localStorage support");
    }
};

// wait for the DOM to load before running the main function
document.addEventListener("DOMContentLoaded", main);
