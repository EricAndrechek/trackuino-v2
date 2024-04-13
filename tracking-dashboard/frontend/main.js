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

const settings = () => {
    // get the settings from the settings form
    console.log("Settings");
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
                        ).toString(),
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
                    const datetime = new Date(
                        Date.parse(data[name]["positions"][i].dt + "Z")
                    ).toString();

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
const main = () => {};

// wait for the DOM to load before running the main function
document.addEventListener("DOMContentLoaded", main);
