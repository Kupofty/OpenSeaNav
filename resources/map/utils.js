/////////////////
/// Constants ///
/////////////////
var EARTH_RADIUS_METERS  = 6378137.0
var METERS_PER_PIXEL_MERCATOR = 156543.03392



/////////////////////////
/// Generic Functions ///
/////////////////////////

//Get Estimated Time of Arrival (seconds)
function getETA(distanceFromBoat, speed) {
    if (speed === 0)
        return NaN;

    return distanceFromBoat/knotsToMps(boatSpeed)
}

//Check position validity (bool)
function isPositionValid(lat, lon) {
    if (isNaN(lat) || isNaN(lon))
        return false

    return ((lat >= -90) && (lat <= 90) && (lon >= -180) && (lon <= 180))
}

//Bearing between 2 positions (°)
function calculateBearing(lat1, lon1, lat2, lon2) {
    lat1 = toRadians(lat1)
    lon1 = toRadians(lon1)
    lat2 = toRadians(lat2)
    lon2 = toRadians(lon2)

    let dLon = lon2 - lon1
    let y = Math.sin(dLon) * Math.cos(lat2)
    let x = Math.cos(lat1) * Math.sin(lat2) -
            Math.sin(lat1) * Math.cos(lat2) * Math.cos(dLon)

    let bearing = toDegrees(Math.atan2(y, x))
    return (bearing + 360) % 360  // Normalize to 0–359°
}

//Distance between 2 positions (meters)
function haversineDistance(lat1, lon1, lat2, lon2) {
    let dLat = toRadians(lat2 - lat1)
    let dLon = toRadians(lon2 - lon1)

    lat1 = toRadians(lat1)
    lat2 = toRadians(lat2)

    let a = Math.sin(dLat / 2) * Math.sin(dLat / 2) +
            Math.cos(lat1) * Math.cos(lat2) *
            Math.sin(dLon / 2) * Math.sin(dLon / 2)

    let c = 2 * Math.atan2(Math.sqrt(a), Math.sqrt(1 - a))

    return EARTH_RADIUS_METERS  * c
}

//Calculate end position when adding distance to initial position/heading (cog/heading line)
function destinationCoordinate(lat, lon, angleDeg, distanceMeters) {
    const brng = Utils.toRadians(angleDeg)
    const phi_1 = lat * Math.PI / 180
    const lambda_1 = lon * Math.PI / 180

    const phi_2 = Math.asin(Math.sin(phi_1) * Math.cos(distanceMeters / EARTH_RADIUS_METERS ) +
                  Math.cos(phi_1) * Math.sin(distanceMeters / EARTH_RADIUS_METERS ) * Math.cos(brng))

    const lambda_2 = lambda_1 + Math.atan2(Math.sin(brng) * Math.sin(distanceMeters / EARTH_RADIUS_METERS ) * Math.cos(phi_1),
                     Math.cos(distanceMeters / EARTH_RADIUS_METERS ) - Math.sin(phi_1) * Math.sin(phi_2))

    return QtPositioning.coordinate(phi_2 * 180 / Math.PI, lambda_2 * 180 / Math.PI)
}

//Calculate map scale bar distance
function metersPerPixel(latitudeDeg, zoomLevel) {
    var latRad = latitudeDeg * Math.PI / 180
    return METERS_PER_PIXEL_MERCATOR * Math.cos(latRad) / Math.pow(2, zoomLevel)
}



///////////////////
/// Conversions ///
///////////////////
function toRadians(deg) {
    return deg * Math.PI / 180.0
}

function toDegrees(rad) {
    return rad * 180.0 / Math.PI
}

function metersToNauticalMiles(meters) {
    return meters / 1852.0
}

function knotsToMps(speedKnots) {
    return speedKnots * 0.514444;
}



//////////////
/// Format ///
//////////////
function formatLat(lat) {
    return Math.abs(lat).toFixed(5) + "°" + (lat >= 0 ? qsTr("N") : qsTr("S"))
}

function formatLon(lon) {
    return Math.abs(lon).toFixed(5) + "°" + (lon >= 0 ? qsTr("E") : qsTr("W"))
}

function secondsToDHMS(seconds) {
    if (isNaN(seconds))
        return qsTr("N/A");

    var secondsInMinute = 60;
    var secondsInHour = 3600;
    var secondsInDay = 86400;

    var d = Math.floor(seconds / secondsInDay)
    var h = Math.floor((seconds % secondsInDay) / secondsInHour)
    var m = Math.floor((seconds % secondsInHour) / secondsInMinute)
    var s = Math.floor(seconds % secondsInMinute)

    // Show days, hours, minutes
    if (d > 0) {
        return (d > 0 ? d + "d " : "") +
               (h > 0 ? h + "h " : "") +
               (m > 0 ? m + "m" : "");
    }

    // Show h, m, s only
    else {
        return (h > 0 ? h + "h " : "") +
               (m > 0 ? m + "m " : "") +
               s + "s";
    }
}

function normalizeAngle180(angleDeg) {
    return ((angleDeg + 180) % 360 + 360) % 360 - 180
}
