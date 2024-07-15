const clientId = "mqttjs_" + Math.random().toString(16).substr(2, 8);

const host = "wss://mqtt.umich-balloons.com:1884/mqtt";

// topics: TELEMETRY, POSITION, ALERT, PREDICTION, SERVER


const parseAPRSSymbol = (symbol) => {
    // take symbol like "/O" and return a string like "Balloon"
    let symbol_table = symbol[0];
    let symbol_code = symbol[1];
    let symbol_string = "";

    if (symbol_table === "/") {
        switch (symbol_code) {
            case "O":
                // should be balloon
                symbol_string = "custom-balloon";
                break;
            case ">":
                symbol_string = "custom-car";
                break;
            default:
                console.log("Unknown symbol: ", symbol);
                symbol_string = "question-mark";
                break;
        }
    } else {
        console.log("Unknown symbol table: ", symbol_table);
        symbol_string = "question-mark";
    }
    return symbol_string;
};

const positionHandler = (topic, payload) => {
    // topic is POSITION/<name>
    // payload is a stringified JSON object with the following keys:
    /*
    name: name of tracked device
    lat: current latitude of tracked device
    lon: current longitude of tracked device
    spd: current speed of device, kmph
    alt: altitude of device, meters
    cse: course/heading of device, degrees
    cmnt: comment about device
    sym: symbol of device
    */

    let name = topic.split("/")[1];
    let data = JSON.parse(payload.toString());

    // check if name exists in positions
    if (!(name in positions)) {
        positions[name] = {
            name: name,
            symbol: parseAPRSSymbol(data.sym),
            last_update: "",
            current: {
                latitude: 0,
                longitude: 0,
                altitude: 0,
                speed: 0,
                course: 90,
                comment: "",
                // TODO: JSON Telemetry data
                datetime: "",
            },
            previous_coordinates: [],
        };
    }

    const heading = parseFloat(data.cse);
    // update image rotation
    if (heading > 180) {
        positions[name].symbol = parseAPRSSymbol(data.sym) + "-flip";
    } else {
        positions[name].symbol = parseAPRSSymbol(data.sym);
    }

    // if lat and lon are both 0, don't update position
    if (data.lat !== 0 && data.lon !== 0) {
        // update the position data
        positions[name].current.latitude = parseFloat(data.lat);
        positions[name].current.longitude = parseFloat(data.lon);
        positions[name].current.altitude = parseFloat(data.alt);
        positions[name].current.speed = parseFloat(data.spd);
        positions[name].current.course = heading;
        positions[name].current.comment = data.cmnt;
        try {
            const parsed_datetime = new Date(Date.parse(data.dt + "Z"));
            positions[name].current.datetime = parsed_datetime.toLocaleString();
        } catch (err) {
            // default to now
            console.log("Error parsing datetime: ", err);
            positions[name].current.datetime = new Date().toLocaleString();
        }

        // and push to previous coordinates
        positions[name].previous_coordinates.push({
            latitude: parseFloat(data.lat),
            longitude: parseFloat(data.lon),
            altitude: parseFloat(data.alt),
            speed: parseFloat(data.spd),
            course: heading,
            comment: data.cmnt,
            datetime: positions[name].current.datetime,
        });

        console.log(
            "Position for ",
            name,
            " updated with datetime: ",
            positions[name].current.datetime
        );
    }

    // check if has telemetry data
    if (name in telemetry) {
        // update last_update in telemetry
        telemetry[name].last_update = positions[name].last_update;
        // update altitude, speed, course in telemetry
        telemetry[name].altitude = data.alt;
        telemetry[name].speed = data.spd;
        telemetry[name].course = data.cse;
    } else {
        // add telemetry data
        if (!(name in telemetry)) {
            telemetry[name] = {
                name: name,
            };
        }
        telemetry[name].altitude = data.alt;
        telemetry[name].speed = data.spd;
        telemetry[name].course = data.cse;
    }

    try {
        const parsed_last_update = new Date(Date.parse(data.dt + "Z"));
        positions[name].last_update = parsed_last_update.toLocaleString();
    } catch (err) {
        // default to now
        console.log("Error parsing last_update: ", err);
        positions[name].last_update = new Date().toLocaleString();
    }
    console.log("Position for ", name, " updated");
};

const telemetryHandler = (topic, payload) => {
    // topic is TELEMETRY/<name>/<key>
    // key/value pairs:
    /*
    name: name of tracked device
    telemetry: telemetry data
        lwt: 0 or 1 (designates if device being tracked is online)
        csq: 0-99 (designates signal strength of the device)
        b%: battery percentage
        bmV: battery millivolts
        bCS: battery charge status
    */

    let name = topic.split("/")[1];
    let key = topic.split("/")[2];
    let value = payload.toString();

    // check if track_id exists in telemetry
    if (!(name in telemetry)) {
        // add track_id to telemetry
        telemetry[name] = {
            name: name,
        };
    }

    // update telemetry data
    if (key === "lwt") {
        console.log("Updating lwt for ", name, " to ", value);
        telemetry[name].lwt = parseInt(value) !== NaN ? parseInt(value) : value;
    } else if (key === "csq") {
        console.log("Updating csq for ", name, " to ", value);
        telemetry[name].csq = parseInt(value) !== NaN ? parseInt(value) : value;
    } else if (key === "b%") {
        console.log("Updating battery for ", name, " to ", value);
        telemetry[name].battery = parseInt(value) !== NaN ? parseInt(value) : value;
    } else if (key === "bmV") {
        console.log("Updating battery_mv for ", name, " to ", value);
        telemetry[name].battery_mv = parseInt(value) !== NaN ? parseInt(value) : value;
    } else if (key === "bCS") {
        console.log("Updating battery_cs for ", name, " to ", value);
        telemetry[name].battery_cs = parseInt(value) !== NaN ? parseInt(value) : value;
    } else {
        // add key/value pair to telemetry
        console.log("Adding key/value pair to telemetry: ", key, value);
        telemetry[name][key] = value;
        // try to parse to float
        try {
            if (parseFloat(value) === NaN) {
                telemetry[name][key] = value;
            } else {
                telemetry[name][key] = parseFloat(value);
            }
        } catch (err) {
            console.log("Error parsing telemetry value: ", err);
        }
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
    needData = true;
});

client.on("disconnect", () => {
    console.log("Client disconnected");
    needData = true;
});

client.on("offline", () => {
    console.log("Client offline");
    notificationBanner("Device offline", 15000);
    needData = true;
});

client.on("end", () => {
    console.log("Client ended");
    needData = true;
});

client.on("reconnect", () => {
    console.log("Reconnecting...");
});

client.on("connect", () => {
    notificationBanner("Online", 5000);

    // we know we are online now and can request old data
    if (needData) {
        requestOldData();
    }

    console.log("Client connected:" + clientId);
    const topics = ["TELEMETRY", "POSITION", "ALERT", "PREDICTION", "SERVER"];
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
    if (topic.includes("POSITION")) {
        positionHandler(topic, payload);
    } else if (topic.includes("TELEMETRY")) {
        console.log("Telemetry message received");
        console.log("Topic: ", topic);
        console.log("Payload: ", payload.toString());
        telemetryHandler(topic, payload);
    } else if (topic.includes("ALERT")) {
        alertHandler(topic, payload);
    } else if (topic.includes("PREDICTION")) {
        predictionHandler(topic, payload);
    } else if (topic.includes("SERVER")) {
        serverHandler(topic, payload);
    } else {
        console.log("Unknown topic: ", topic);
    }
});
