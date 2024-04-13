const CACHE_NAME = "balloon-map-cache-v1.0.0";
const urlsToCache = [
    "/",
    "/main.js",
    "/styles.css",
    "/mqtt.js",
    "/manifest.json",
    "https://api.mapbox.com/mapbox-gl-js/v3.2.0/mapbox-gl.css",
    "https://api.mapbox.com/mapbox-gl-js/v3.2.0/mapbox-gl.js",
    "https://unpkg.com/mqtt@5.5.1/dist/mqtt.min.js",
    "https://api.mapbox.com/styles/v1/ericandr/cluvydoes006n01pk7a9z9hwv?sdk=js-3.2.0&access_token=pk.eyJ1IjoiZXJpY2FuZHIiLCJhIjoiY2x1cnp3Z2tpMDcybTJycGVwN2czNTk3OCJ9.Ccv9oGIfUewgj2Iw-CqOyw",
    "https://api.mapbox.com/styles/v1/ericandr/cluvy4r03005g01nrdfgg89ea?sdk=js-3.2.0&access_token=pk.eyJ1IjoiZXJpY2FuZHIiLCJhIjoiY2x1cnp3Z2tpMDcybTJycGVwN2czNTk3OCJ9.Ccv9oGIfUewgj2Iw-CqOyw",
    "https://api.mapbox.com/v4/mapbox.mapbox-terrain-dem-v1.json?secure&access_token=pk.eyJ1IjoiZXJpY2FuZHIiLCJhIjoiY2x1cnp3Z2tpMDcybTJycGVwN2czNTk3OCJ9.Ccv9oGIfUewgj2Iw-CqOyw",
    "https://api.mapbox.com/v4/mapbox.mapbox-streets-v8,mapbox.mapbox-terrain-v2,mapbox.mapbox-bathymetry-v2.json?secure&access_token=pk.eyJ1IjoiZXJpY2FuZHIiLCJhIjoiY2x1cnp3Z2tpMDcybTJycGVwN2czNTk3OCJ9.Ccv9oGIfUewgj2Iw-CqOyw",
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
