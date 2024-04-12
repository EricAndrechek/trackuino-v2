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
                course: 0,
                comment: "",
                datetime: "",
            },
            previous_coordinates: [],
        };
    } else {
        // add previous coordinates to previous_coordinates
        positions[name].previous_coordinates.push({
            latitude: positions[name].current.latitude,
            longitude: positions[name].current.longitude,
            altitude: positions[name].current.altitude,
            speed: positions[name].current.speed,
            course: positions[name].current.course,
            comment: positions[name].current.comment,
            datetime: positions[name].current.datetime,
        });
    }

    const heading = parseFloat(data.cse);
    // update image rotation
    if (heading > 180) {
        positions[name].symbol = parseAPRSSymbol(data.sym) + "-flip";
    } else {
        positions[name].symbol = parseAPRSSymbol(data.sym);
    }

    // update the position data
    positions[name].current.latitude = parseFloat(data.lat);
    positions[name].current.longitude = parseFloat(data.lon);
    positions[name].current.altitude = parseFloat(data.alt);
    positions[name].current.speed = parseFloat(data.spd);
    positions[name].current.course = heading;
    positions[name].current.comment = data.cmnt;
    try {
        positions[name].current.datetime = new Date(
            Date.parse(data.dt),
            "UTC"
        ).toISOString();
    } catch (err) {
        // default to now
        positions[name].current.datetime = new Date().toISOString();
    }

    // check if has telemetry data
    if (name in telemetry) {
        // update last_update in telemetry
        telemetry[name].last_update = new Date().toISOString();
        // update altitude, speed, course in telemetry
        telemetry[name].altitude = data.alt;
        telemetry[name].speed = data.spd;
        telemetry[name].course = data.cse;
    } else {
        // add telemetry data
        telemetry[name] = {
            name: name,
            lwt: 0,
            csq: 0,
            battery: 0,
            battery_mv: 0,
            battery_cs: 0,
            speed: data.spd,
            course: data.cse,
            altitude: data.alt,
            last_update: new Date().toISOString(),
        };
    }

    try {
        positions[name].last_update = new Date(
            Date.parse(data.dt),
            "UTC"
        ).toISOString();
    } catch (err) {
        // default to now
        positions[name].last_update = new Date().toISOString();
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
            lwt: 0,
            csq: 0,
            battery: 0,
            battery_mv: 0,
            battery_cs: 0,
            speed: 0,
            course: 0,
            altitude: 0,
            last_update: {
                hh: 0,
                mm: 0,
                ss: 0,
                MM: 0,
                DD: 0,
                YY: 0,
            },
        };
    }

    // update telemetry data
    if (key === "lwt") {
        telemetry[name].lwt = parseInt(value);
    } else if (key === "csq") {
        telemetry[name].csq = parseInt(value);
    } else if (key === "b%") {
        telemetry[name].battery = parseInt(value);
    } else if (key === "bmV") {
        telemetry[name].battery_mv = parseInt(value);
    } else if (key === "bCS") {
        telemetry[name].battery_cs = parseInt(value);
    } else {
        // add key/value pair to telemetry
        telemetry[name][key] = value;
        // try to parse to float
        try {
            telemetry[name][key] = parseFloat(value);
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
    notificationBanner("Online and connected to server", 5000);

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
