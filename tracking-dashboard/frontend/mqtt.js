const clientId = "mqttjs_" + Math.random().toString(16).substr(2, 8);

const host = "wss://mqtt.umich-balloons.com:1884/mqtt";

// topics: T (track), A (alert), P (prediction), S (server), B (balloon)
// balloon is used to track a balloon assigned to a specific track id, not for our usage

const trackHandler = (topic, payload) => {
    // topic is T/<track_id>/<key>
    // key/value pairs:
    /*
    lwt: 0 or 1 (designates if device being tracked is online)
    csq: 0-99 (designates signal strength of the device)
    lat: current latitude of tracked device
    lon: current longitude of tracked device
    spd: current speed of device, kmph
    alt: altitude of device, meters
    cse: course/heading of device, degrees
    b%: battery percentage
    bmV: battery millivolts
    bCS: battery charge status
    hh: hours
    mm: minutes
    ss: seconds
    MM: months
    DD: days
    YY: years
    */

    let track_id = topic.split("/")[1];
    let key = topic.split("/")[2];
    let value = payload.toString();

    // check if track_id exists in tracks
    if (!(track_id in tracks)) {
        // check if track_id exists in id_map
        if (!(track_id in id_map)) {
            console.log("Unknown track id ", track_id, " received");
            id_map[track_id] = {
                name: "Unknown",
                type: "Unknown",
            };
        }
        // add track_id to tracks
        tracks[track_id] = {
            id: track_id,
            name: id_map[track_id].name,
            type: id_map[track_id].type,
            lat: 0,
            lon: 0,
            alt: 0,
            spd: 0,
            cse: 0,
            battery: 0,
            battery_mv: 0,
            battery_cs: 0,
            last_update: {
                hh: 0,
                mm: 0,
                ss: 0,
                MM: 0,
                DD: 0,
                YY: 0,
            },
            geojson: {
                type: "Feature",
                geometry: {
                    type: "LineString",
                    coordinates: [],
                },
                properties: {
                    name: "",
                    id: track_id,
                    last_update: {
                        hh: 0,
                        mm: 0,
                        ss: 0,
                        MM: 0,
                        DD: 0,
                        YY: 0,
                    },
                },
            },
        };
    }

    // if lat, lon, or alt, add to geojson coordinates
    if (key === "lat" || key === "lon" || key === "alt") {
        tracks[track_id].geojson.geometry.coordinates.push([
            parseFloat(tracks[track_id].lon),
            parseFloat(tracks[track_id].lat),
            parseFloat(tracks[track_id].alt),
        ]);
        console.log("Track id ", track_id, " updated with new coordinates");
    }

    // update the track data
    if (key === "lwt") {
        tracks[track_id].lwt = value;
    } else if (key === "csq") {
        tracks[track_id].csq = value;
    } else if (key === "lat") {
        tracks[track_id].lat = value;
    } else if (key === "lon") {
        tracks[track_id].lon = value;
    } else if (key === "spd") {
        tracks[track_id].spd = value;
    } else if (key === "alt") {
        tracks[track_id].alt = value;
    } else if (key === "cse") {
        tracks[track_id].cse = value;
    } else if (key === "b%") {
        tracks[track_id].battery = value;
    } else if (key === "bmV") {
        tracks[track_id].battery_mv = value;
    } else if (key === "bCS") {
        tracks[track_id].battery_cs = value;
    } else if (key === "hh") {
        tracks[track_id].last_update.hh = value;
    } else if (key === "mm") {
        tracks[track_id].last_update.mm = value;
    } else if (key === "ss") {
        tracks[track_id].last_update.ss = value;
    } else if (key === "MM") {
        tracks[track_id].last_update.MM = value;
    } else if (key === "DD") {
        tracks[track_id].last_update.DD = value;
    } else if (key === "YY") {
        tracks[track_id].last_update.YY = value;
    } else {
        console.log("Unknown key ", key, " received for track id ", track_id);
    }
};

const alertHandler = (topic, payload) => {
    // topic is A/<alert_id>
    console.log(
        "Alert message received with id: ",
        topic.split("/")[1],
        " and message: ",
        payload.toString()
    );
    notificationBanner(payload.toString(), 5000);
};

const predictionHandler = (topic, payload) => {};

const serverHandler = (topic, payload) => {
    // TODO - not sure what to do with this yet, but adding it so we can use it later if needed
};

const options = {
    keepalive: 60,
    clientId: clientId,
    protocolId: "MQTT",
    protocolVersion: 5,
    clean: true,
    reconnectPeriod: 1000,
    connectTimeout: 10 * 1000,
};

console.log("Connecting mqtt client");
const client = mqtt.connect(host, options);

client.on("error", (err) => {
    console.log("Connection error: ", err);
    client.end();
});

client.on("close", () => {
    console.log("Client closed");
});

client.on("disconnect", () => {
    console.log("Client disconnected");
});

client.on("offline", () => {
    console.log("Client offline");
    notificationBanner("Device offline", 15000);
});

client.on("end", () => {
    console.log("Client ended");
});

client.on("reconnect", () => {
    console.log("Reconnecting...");
});

client.on("connect", () => {
    notificationBanner("Online and connected to server", 5000);
    console.log("Client connected:" + clientId);
    // Subscribe to topics T, A, P, S
    const topics = ["T", "A", "P", "S"];
    topics.forEach((topic) => {
        let topic_string = topic + "/#";
        client.subscribe(topic_string, { qos: 1 }, (err) => {
            if (err) {
                console.log("Subscribe error: ", err);
            } else {
                console.log("Subscribed to: ", topic);
            }
        });
    });
});

client.on("message", (topic, payload) => {
    // establish if topic is T (track), A (alert), P (prediction), S (server)
    // and get the id (if any) from the topic
    let topic_type = topic[0];
    if (topic_type === "T") {
        trackHandler(topic, payload);
    } else if (topic_type === "A") {
        alertHandler(topic, payload);
    } else if (topic_type === "P") {
        predictionHandler(topic, payload);
    } else if (topic_type === "S") {
        serverHandler(topic, payload);
    } else {
        console.log(
            "Unknown message topic ",
            topic,
            " received: ",
            payload.toString()
        );
    }
});
