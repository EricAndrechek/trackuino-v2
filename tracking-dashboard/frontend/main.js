// id to name and type mapping
let id_map = {};

// track storage object
let tracks = {};
// track data:
// key: id
// value: {
//     id: id,
//     name: name,
//     type: type,
//     lat: lat,
//     lon: lon,
//     alt: alt,
//     spd: spd,
//     cse: cse,
//     battery: b%,
//     battery_mv: bmV,
//     battery_cs: bCS,
//     last_update: {
//         hh: hh,
//         mm: mm,
//         ss: ss,
//         MM: MM,
//         DD: DD,
//         YY: YY,
//     },
//     geojson: {
//         type: "Feature",
//         geometry: {
//             type: "LineString",
//             coordinates: [],
//         },
//         properties: {
//             name: name,
//             id: id,
//             last_update: {
//                 hh: hh,
//                 mm: mm,
//                 ss: ss,
//                 MM: MM,
//                 DD: DD,
//                 YY: YY,
//             },
//         },
//     },
// };

// prediction storage object
let predictions = {};

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
const main = () => {
    // get api.umich-balloons.com/api/liveTracks
    fetch("https://api.umich-balloons.com/api/liveTracks")
        .then((response) => response.json())
        .then((data) => {
            // data will be an array of objects with key as id and value as object with name and type
            id_map = data;
        })
        .catch((error) => {
            console.error("Error loading tracks data: ", error);
        });
};

// wait for the DOM to load before running the main function
document.addEventListener("DOMContentLoaded", main);

console.log("Main script loaded");
