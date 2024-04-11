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

const getHistoricalGeoJSON = (name) => {
    let coordinates = [];
    if (name in positions) {
        for (let i = 0; i < positions[name].previous_coordinates.length; i++) {
            coordinates.push([
                positions[name].previous_coordinates[i].longitude,
                positions[name].previous_coordinates[i].latitude,
                positions[name].previous_coordinates[i].altitude,
            ]);
        }
    }
    return {
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

// main function
const main = () => {};

// wait for the DOM to load before running the main function
document.addEventListener("DOMContentLoaded", main);
