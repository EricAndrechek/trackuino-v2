const CACHE_NAME = "balloon-map-cache-v1.0.6";
const urlsToCache = [
    "/",
    "/main.js",
    "/styles.css",
    "/mqtt.js",
    "/manifest.json",
    "/assets/balloon.png",
    "/assets/car.png",
    "/assets/car-flip.png",
    "/assets/chevron-down.svg",
    "/assets/map-pin.svg",
    "/assets/settings.svg",
    "/assets/x-mark.svg",
    "https://api.mapbox.com/mapbox-gl-js/v3.2.0/mapbox-gl.css",
    "https://api.mapbox.com/mapbox-gl-js/v3.2.0/mapbox-gl.js",
    "https://unpkg.com/mqtt@5.5.1/dist/mqtt.min.js",
    "https://unpkg.com/@turf/turf@6/turf.min.js",
];

self.addEventListener("install", (event) => {
    console.log("Service Worker installed");
    self.skipWaiting(); // Activate worker immediately
});

self.addEventListener("activate", (event) => {
    console.log("Service Worker activated");
    event.waitUntil(
        caches.keys().then((cacheNames) => {
            return Promise.all(
                cacheNames.map((cacheName) => {
                    if (cacheName !== CACHE_NAME) {
                        console.log("Service Worker: Clearing old cache");
                        return caches.delete(cacheName);
                    }
                })
            );
        })
    );
});

self.addEventListener("fetch", (event) => {
    // don't cache MQTT requests, http, extensions, etc
    let url = event.request.url;
    if (
        url.startsWith('chrome-extension') ||
        url.includes('extension') ||
        !(url.indexOf('http') === 0) ||
        url.includes('mqtt') ||
        url.includes('socket') ||
        url.includes('chrome-extension')
    ) {
        return;
    }

    event.respondWith(
        fetch(event.request)
            .then((res) => {
                // Make copy/clone of response
                const resClone = res.clone();
                // Open cache
                caches.open(CACHE_NAME).then((cache) => {
                    // Add response to cache
                    cache.put(event.request, resClone);
                });
                return res;
            })
            .catch((err) => caches.match(event.request).then((res) => res))
    );
});
