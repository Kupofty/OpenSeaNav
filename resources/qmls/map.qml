
////////////////////////
/// Import libraries ///
////////////////////////
import QtQuick
import QtQuick.Controls
import QtLocation
import QtPositioning



Item {
    id: window

    ////////////////////////
    /// Global variables ///
    ////////////////////////

    //Global Variables
    property double earthRadiusMeters: 6378137.0;
    property double metersPerPixelMercatorProjection : 156543.03392

    //Position
    property double mapCenterInitLatitude:  35
    property double mapCenterInitLongitude: 0
    property var currentBoatCoord: null
    property bool boatPositionInit: false

    //Cursor
    property var cursorCoord: NaN
    property double cursorLatitude: NaN
    property double cursorLongitude: NaN
    property double cursorDistanceBoat: NaN
    property double cursorBearingBoat: NaN

    //Boat data
    property string noData: qsTr("No Data")
    property string boatDate: ""
    property string boatTime: ""
    property double boatLatitude: 0
    property double boatLongitude: 0
    property double boatHeading: 0          //°
    property double boatCourse: 0           //°
    property double boatDepth: 0            //meters
    property double boatSpeed: 0            //knots
    property double boatWaterTemperature: 0 //°C
    property double satellitesInView: 0

    //Boat data received check
    property bool boatDateReceived: false
    property bool boatTimeReceived: false
    property bool boatPositionReceived: false
    property bool boatHeadingReceived: false
    property bool boatCourseReceived: false
    property bool boatDepthReceived: false
    property bool boatSpeedReceived: false
    property bool boatWaterTemperatureReceived: false
    property bool satellitesReceived: false

    //Labels
    property int rightClickMenuWidth: 150
    property int labelRightSideWidth: 135
    property int labelLeftSideWidth: 150
    property int labelPadding: 8
    property int labelLateralMargin: 8
    property int labelVerticalMargin: 8
    property int labelFontSize: 14
    property int labelBackgroundRadius: 4
    property string labelColor: "white"
    property string labelBackgroundColor: "grey"

    //Zoom
    property double mapZoomLevelInit: 3
    property double zoomIncrement: 0.2

    //Markers
    property int userMarkerCount: 0
    property var boatMarkerRef: null

    //Map
    property bool showUI: true
    property bool followBoat: false

    // View modes
    property double wheelDragSensitivity: 0.25
    property int mapViewMode: 0
    property double freeViewUp: 0
    property real mapRotation: {
        switch (mapViewMode)
        {
            case 0: default: return 0   // North Up
            case 1: return boatHeading  // Heading Up
            case 2: return boatCourse   // COG Up
            case 3: return freeViewUp   // Free view
        }
    }

    //Timer Data Update
    property int timeBeforePositionLost: 10   //seconds
    property int timeBeforeGeneralDataLost: 5 //seconds
    property string textTimerPositionUpdate: qsTr("No Position Data")

    property double timeLastUtcDate: 0
    property double timeLastUtcTime: 0
    property double timeLastPosition: 0
    property double timeLastHeading: 0
    property double timeLastCourse: 0
    property double timeLastSpeed: 0
    property double timeLastDepth: 0
    property double timeLastWaterTemp: 0
    property double timeLastSatellites: 0
    property double elapsedSec: 0

    //Heading & COG lines
    property int distanceLineTimeTrip: 300 //seconds
    property int cogLineDistance: knotsToMps(boatSpeed) * distanceLineTimeTrip
    property real headingLineDistance: knotsToMps(boatSpeed) * distanceLineTimeTrip * Math.cos(toRadians(boatCourse - boatHeading))


    //Boat Track
    property bool enableTrack: false
    property int minimumTrackPointsDistance: 50 //meters
    property var boatTrack: []
    property int maxTrackPoints: 500
    property int trackLineWidth: 10
    property double trackLineOpacity: 0.6
    property color trackLineColor: Qt.rgba(1, 1, 0, trackLineOpacity)

    //Measure mode
    property bool measureMode: false
    property var measurePoint: null
    property var measureTrack: []
    property real measureTotalMeters: 0

    //Map overlay
    property double mapOverlayOpacity: 1

    //-------------------------------------------------------------------------------//


    //////////////////
    /// OSM Plugin ///
    //////////////////

    //OpenStreetMaps sources
    Plugin {
        id: osmPlugin
        name: "osm"

        PluginParameter {
            name: "osm.mapping.host"
            value: "https://a.tile.opentopomap.org/"
        }

        //Disable Qt's default provider
        PluginParameter {
           name: "osm.mapping.providersrepository.disabled"
           value: true
        }
    }

    //OpenSeaMap overlay source
    Plugin {
        id: openSeaMapPlugin
        name: "osm"

        PluginParameter {
            name: "osm.mapping.custom.host"
            value: "https://tiles.openseamap.org/seamark/"
        }

        PluginParameter {
            name: "osm.mapping.custom.type"
            value: "png"
        }

        PluginParameter {
            name: "osm.mapping.providersrepository.disabled"
            value: true
        }
    }


    ////////////
    /// Maps ///
    ////////////

    //Main map
    Map {
        id: map
        anchors.fill: parent
        center: QtPositioning.coordinate(mapCenterInitLatitude, mapCenterInitLongitude)
        zoomLevel: mapZoomLevelInit
        activeMapType: map.supportedMapTypes[map.supportedMapTypes.length - 1]
        bearing: mapRotation

        //Load OSM plugin
        plugin: osmPlugin

        //Track Line
        MapPolyline { //Lines between existing waypoints
            visible: boatTrack.length > 1
            line.width: trackLineWidth
            line.color: trackLineColor
            path: boatTrack
        }
        MapPolyline { //Line from last waypoint to boat
            line.width: trackLineWidth
            line.color: trackLineColor

            path: {
                if (!enableTrack)
                    return []
                if (boatTrack.length === 0)
                    return []
                if (!currentBoatCoord)
                    return []

                return [
                    boatTrack[boatTrack.length - 1],
                    currentBoatCoord
                ]
            }
        }

        //COG Line
        MapPolyline {
            visible: (boatPositionReceived && boatCourseReceived)
            line.width: 3
            line.color: "blue"

            path: [
                QtPositioning.coordinate(boatLatitude, boatLongitude),
                destinationCoordinate(boatLatitude, boatLongitude, boatCourse, cogLineDistance)
            ]
        }
        MapQuickItem {
            visible: (boatPositionReceived && boatCourseReceived)
            coordinate: destinationCoordinate(boatLatitude, boatLongitude, boatCourse, cogLineDistance)
            anchorPoint.x: 5
            anchorPoint.y: 5
            sourceItem: Rectangle {
                width: 10
                height: 10
                radius: 5
                color: "blue"
                border.color: "black"
                border.width: 1
            }
        }

        //Heading Line
        MapPolyline {
            visible: (boatPositionReceived && boatHeadingReceived)
            line.width: 2
            line.color: "red"

            path: [
                QtPositioning.coordinate(boatLatitude, boatLongitude),
                destinationCoordinate(boatLatitude, boatLongitude, boatHeading, headingLineDistance)
            ]
        }
        MapQuickItem {
            visible: (boatPositionReceived && boatHeadingReceived)
            coordinate: destinationCoordinate(boatLatitude, boatLongitude, boatHeading, headingLineDistance)
            anchorPoint.x: 3
            anchorPoint.y: 3
            sourceItem: Rectangle {
                width: 10
                height: 10
                radius: 5
                color: "red"
                border.color: "black"
                border.width: 1
            }
        }

        //Measure Distance Line
        MapPolyline {
            visible: measureMode && measureTrack.length > 0
            line.width: 2
            line.color: "black"

            path: {
                if (measureTrack.length === 0) return []
                var last = measureTrack[measureTrack.length - 1]
                var current = cursorCoord
                return [last, current]
            }
        }
        MapPolyline {
            visible: measureMode && measureTrack.length > 1
            line.width: 2
            line.color: "black"

            path: measureTrack.length > 0 ? measureTrack : []
        }
    }

    // Overlay
    Map {
        id: openSeaMapLayer
        anchors.fill: parent
        plugin: openSeaMapPlugin

        // Sync camera with base map
        center: map.center
        zoomLevel: map.zoomLevel
        bearing: map.bearing
        tilt: map.tilt

        // Overlay behavior
        opacity: mapOverlayOpacity
        color: "transparent"

        // Force custom map type
        activeMapType: supportedMapTypes.find(
            t => t.style === MapType.CustomMap
        )
    }



    //////////////////////////
    /// Keyboard Shortcuts ///
    //////////////////////////
    Shortcut { //Follow boat
        sequence: "F"
        onActivated: followBoat = !followBoat
        enabled: boatPositionReceived
    }

    Shortcut { //Close View
        sequence: "Ctrl++"
        onActivated: goToZoomLevelMap(17)
    }

    Shortcut { //Wide View
        sequence: "Ctrl+-"
        onActivated: goToZoomLevelMap(12)
    }

    Shortcut { //Zoom In
        sequence: "+"
        onActivated: goToZoomLevelMap(map.zoomLevel + 1)
    }

    Shortcut { //Zoom out
        sequence: "-"
        onActivated: goToZoomLevelMap(map.zoomLevel - 1)
    }

    Shortcut { //Hide UI
        sequence: "H"
        onActivated: showUI = !showUI
    }

    Shortcut { //Enter Measure Mode
        sequence: "D"
        onActivated: measureMode = !measureMode
    }

    Shortcut { //Quit Measure Mode
        sequence: "Escape"
        onActivated: {
            if(measureMode){
                measureMode = false
                measurePoint = null
            }
        }
    }

    Shortcut { //Switch View Up
        sequence: "V"
        onActivated: {
            if(mapViewMode == 3) //return to north up if in free view
                mapViewMode = 0
            else                 //cycle through north/heading/course
                mapViewMode = (mapViewMode + 1) % 3
        }
    }

    Shortcut { //Drop Marker On Boat
        sequence: "M"
        onActivated: addMarkerOnMap(boatLatitude, boatLongitude)
    }

    Shortcut { //Enable Tracking
        sequence: "T"
        onActivated: enableTrack = !enableTrack
    }

    Shortcut { //Clear Track
        sequence: "C"
        onActivated: boatTrack = []
    }



    ////////////////////
    /// WheelHandler ///
    ////////////////////
    WheelHandler {
        target: map

        onWheel: function(event) { //Zoom on cursor
            //Coordinates before zoom
            var cursorPoint = Qt.point(event.x, event.y)
            var cursorCoordBefore = map.toCoordinate(cursorPoint)

            //Zoom
            if (event.angleDelta.y > 0)
                map.zoomLevel = Math.min(map.maximumZoomLevel, map.zoomLevel + zoomIncrement)
            else if (event.angleDelta.y < 0)
                map.zoomLevel = Math.max(map.minimumZoomLevel, map.zoomLevel - zoomIncrement)

            //Coordinates after zoom
            var cursorCoordAfter = map.toCoordinate(cursorPoint)
            var latShift = cursorCoordBefore.latitude - cursorCoordAfter.latitude
            var lonShift = cursorCoordBefore.longitude - cursorCoordAfter.longitude

            // Adjust map center so cursor stays fixed
            map.center = QtPositioning.coordinate(map.center.latitude + latShift, map.center.longitude + lonShift)
        }
    }



    //////////////////
    /// Mouse Area ///
    //////////////////
    MouseArea {
        id: mouseArea
        anchors.fill: map
        hoverEnabled: true
        acceptedButtons: Qt.LeftButton | Qt.RightButton | Qt.MiddleButton

        property var lastCoord
        property bool dragging: false

        property real lastX
        property bool rotating: false

        // Click buttons
        onClicked: function(mouse) {
            //Right-click
            if (mouse.button === Qt.RightButton) {
                if (measureMode) {
                    measureMode = false
                    measurePoint = null
                    measureTrack = []
                    measureTotalMeters = 0
                }
                else {
                    contextMenu.popup()
                }
            }
        }

        //Mouse button pressed
        onPressed: function(mouse) {
            //Left press
            if (mouse.button === Qt.LeftButton) {
                if (measureMode) {
                    var coord = map.toCoordinate(Qt.point(mouse.x, mouse.y))
                    if (measureTrack.length > 0) {
                        var last = measureTrack[measureTrack.length - 1]
                        measureTotalMeters += haversineDistance(
                            last.latitude, last.longitude,
                            coord.latitude, coord.longitude
                        )
                    }

                    // Reassign to a new array to trigger QML binding
                    measureTrack = measureTrack.concat([coord])
                    measurePoint = measureTrack[0]
                }
                else
                {
                    lastCoord = map.toCoordinate(Qt.point(mouse.x, mouse.y))
                    dragging = true
                }
            }

            // Middle button
            else if (mouse.button === Qt.MiddleButton) { //map rotating
                rotating = true
                lastX = mouse.x
            }
        }

        //Mouse button released
        onReleased: function(mouse) {
            dragging = false
            rotating = false
        }

        //Moving mouse
        onPositionChanged: function(mouse) {
            cursorCoord = map.toCoordinate(Qt.point(mouse.x, mouse.y))

            // Free View - rotate map
            if (rotating) {
                mapViewMode = 3
                freeViewUp += (mouse.x - lastX) * wheelDragSensitivity
                lastX = mouse.x
                return
            }

            //Map panning
            if (!measureMode && dragging && mouse.buttons === Qt.LeftButton) {
                var currentCoord = map.toCoordinate(Qt.point(mouse.x, mouse.y))
                var dx = lastCoord.longitude - currentCoord.longitude
                var dy = lastCoord.latitude - currentCoord.latitude
                map.center = QtPositioning.coordinate(map.center.latitude + dy, map.center.longitude + dx)
                lastCoord = map.toCoordinate(Qt.point(mouse.x, mouse.y))
            }
            else
                lastCoord = map.toCoordinate(Qt.point(mouse.x, mouse.y))

            // Cursor info
            cursorLatitude = cursorCoord.latitude
            cursorLongitude = cursorCoord.longitude
            cursorDistanceBoat = haversineDistance(boatLatitude, boatLongitude, cursorLatitude, cursorLongitude)
            cursorBearingBoat = calculateBearing(boatLatitude, boatLongitude, cursorLatitude, cursorLongitude)
        }

        //Text next to cursor (in measure mode only)
        Text {
            x: mouseArea.mouseX + 10
            y: mouseArea.mouseY + 10
            font.pixelSize: 14
            color: "black"
            z: 999

            visible: measureMode && measureTrack.length !== 0

            text: {
                if (measureTrack.length === 0)
                    return ""

                // Distance and bearing from last waypoint to cursor
                var last = measureTrack[measureTrack.length - 1]
                var cursor = cursorCoord
                var distMeters = haversineDistance(
                    last.latitude, last.longitude,
                    cursor.latitude, cursor.longitude
                )
                var distStr = distMeters < 1000
                    ? Math.round(distMeters) + " m"
                    : (distMeters / 1000).toFixed(2) + " km"
                var distNM = metersToNauticalMiles(distMeters).toFixed(2) + " NM"
                var bearingStr = calculateBearing(
                    last.latitude, last.longitude,
                    cursor.latitude, cursor.longitude
                ).toFixed(0) + "°"

                // Compute total track distance (between waypoints + last waypoint to cursor)
                var trackMeters = distMeters
                for (var i = 1; i < measureTrack.length; ++i) {
                    trackMeters += haversineDistance(
                        measureTrack[i-1].latitude, measureTrack[i-1].longitude,
                        measureTrack[i].latitude, measureTrack[i].longitude
                    )
                }
                var trackStr = trackMeters < 1000
                    ? Math.round(trackMeters) + " m"
                    : (trackMeters / 1000).toFixed(2) + " km"
                var trackNM = metersToNauticalMiles(trackMeters).toFixed(2) + " NM"

                //Text
                return qsTr("Track Total: ") + trackStr + " / " + trackNM + "\n"
                     + qsTr("To Cursor: ") + distStr + " / " + distNM + "\n"
                     + qsTr("Bearing: ") + bearingStr
            }
        }
    }



    ////////////////////////
    /// Right-Click Menu ///
    ////////////////////////

    //// Menu ////
    Menu {
        id: contextMenu
        modal: true
        width: rightClickMenuWidth

        //Center view
        MenuItem{
            id: centerViewItem

            contentItem: Label {
                text: qsTr("Center View...")
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                width: parent.width
            }

            onTriggered: {
                centerViewSubmenu.popup(contextMenu.x + contextMenu.width, contextMenu.y + centerViewItem.y)
            }
        }

        //Follow boat
        MenuItem {
            id: followBoatItem

            enabled: boatPositionReceived

            contentItem: Label {
                text: (followBoat ? qsTr("Unfollow Boat") : qsTr("Follow Boat")) + " (F)"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                width: parent.width
            }

            onTriggered: {
                followBoat = !followBoat
            }
        }

        //Draw Track
        MenuItem {
            id: drawTrackItem
            contentItem: Label {
                text: qsTr("Tracking...")
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                width: parent.width
            }

            onTriggered: {
                boatTrackingSubmenu.popup(contextMenu.x + contextMenu.width, contextMenu.y + drawTrackItem.y)
            }
        }

        //Zoom
        MenuItem {
            id: zoomItem

            contentItem: Label {
                text: "Zoom..."
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                width: parent.width
            }

            onTriggered: {
                zoomSubmenu.popup(contextMenu.x + contextMenu.width, contextMenu.y + zoomItem.y)
            }
        }

        //View up
        MenuItem {
            id: viewUpItem

            contentItem: Label {
                text: qsTr("View Up...") + " (V)"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                width: parent.width
            }

            onTriggered: {
                viewUpSubmenu.popup(contextMenu.x + contextMenu.width, contextMenu.y + viewUpItem.y)
            }
        }

        //Markers
        MenuItem{
            id: markersItem

            contentItem: Label {
                text: qsTr("Markers...")
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                width: parent.width
            }

            onTriggered: {
                markerSubmenu.popup(contextMenu.x + contextMenu.width, contextMenu.y + markersItem.y)
            }
        }

        //UI Visibility
        MenuItem {
            id: uiVisibilityItem

            contentItem: Label {
                text: (showUI ? qsTr("Hide UI") : qsTr("Show UI")) + " (H)"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                width: parent.width
            }

            onTriggered: {
                showUI = !showUI
            }
        }

        //Measure mode
        MenuItem {
            id: measureModeItem
            contentItem: Label {
                text: (measureMode ? qsTr("Stop Measuring") : qsTr("Measure Distance")) + " (D)"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                width: parent.width
            }

            onTriggered: {
                measureMode = !measureMode
                if (!measureMode) {
                    measurePoint = null
                }
            }
        }
    }


    //// Submenus ////
    //Center View
    Menu {
        id: centerViewSubmenu
        width: rightClickMenuWidth/1.2
        modal: true

        MenuItem {
            enabled: boatPositionInit

            contentItem: Label {
                text: qsTr("On Boat")
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                width: parent.width
            }

            onTriggered: {
                setCenterPositionOnBoat()
                goToZoomLevelMap(15)
            }
        }

        MenuItem {
            contentItem: Label {
                text: qsTr("On Cursor")
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                width: parent.width
            }
            onTriggered: setCenterPosition(cursorLatitude, cursorLongitude)
        }

        MenuItem {
            contentItem: Label {
                text: qsTr("On Position")
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                width: parent.width
            }
            onTriggered: centerViewDialog.open()
        }
    }

    //Zoom
    Menu {
        id: zoomSubmenu
        width: rightClickMenuWidth
        modal: true

        MenuItem {
            contentItem: Label {
                text: qsTr("Maximum Zoom")
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                width: parent.width
            }
            onTriggered:{
                goToZoomLevelMap(map.maximumZoomLevel)
            }
        }

        MenuItem {
            contentItem: Label {
                text: qsTr("Minimum Zoom")
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                width: parent.width
            }
            onTriggered:{
                goToZoomLevelMap(map.minimumZoomLevel)
            }
        }

        MenuItem {
            contentItem: Label {
                text: qsTr("Close View") + " (Ctrl+)"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                width: parent.width
            }
            onTriggered:{
                goToZoomLevelMap(17)
            }
        }

        MenuItem {
            contentItem: Label {
                text: qsTr("Wide View") + " (Ctrl-)"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                width: parent.width
            }
            onTriggered:{
                goToZoomLevelMap(12)
            }
        }

        MenuItem {
            contentItem: Label {
                text: qsTr("Zoom In") + " (+)"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                width: parent.width
            }
            onTriggered:{
                goToZoomLevelMap(map.zoomLevel+1)
            }
        }

        MenuItem {
            contentItem: Label {
                text: qsTr("Zoom Out") + " (-)"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                width: parent.width
            }
            onTriggered:{
                goToZoomLevelMap(map.zoomLevel-1)
            }
        }
    }

    //View Up
    Menu {
        id: viewUpSubmenu
        width: rightClickMenuWidth/1.2
        modal: true

        MenuItem {
            contentItem: Label {
                text: qsTr("North Up")
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                width: parent.width
            }
            onTriggered: mapViewMode = 0
        }

        MenuItem {
            contentItem: Label {
                text: qsTr("Heading Up")
                enabled: boatHeadingReceived
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                width: parent.width
            }
            onTriggered: mapViewMode = 1
        }

        MenuItem {
            contentItem: Label {
                text: qsTr("Course Up")
                enabled: boatCourseReceived
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                width: parent.width
            }
            onTriggered: mapViewMode = 2
        }
    }

    //Markers
    Menu {
        id: markerSubmenu
        width: rightClickMenuWidth/0.8
        modal: true

        MenuItem {
            contentItem: Label {
                text: qsTr("Drop Marker On Cursor")
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                width: parent.width
            }
            onTriggered: addMarkerOnMap(cursorLatitude, cursorLongitude)
        }

        MenuItem{
            enabled: boatPositionInit

            contentItem: Label {
                text: qsTr("Drop Marker On Boat") + " (M)"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                width: parent.width
            }
            onTriggered: {
                addMarkerOnMap(boatLatitude, boatLongitude)
            }
        }

        MenuItem {
            contentItem: Label {
                text: qsTr("Drop Marker On Position")
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                width: parent.width
            }
            onTriggered: dropMarkerDialog.open()
        }

        MenuItem {
            id: clearMarkersItem
            enabled: userMarkerCount > 0
            contentItem: Label {
                text: qsTr("Clear Markers")
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                width: parent.width
            }
            onTriggered: clearMarkers()
        }
    }

    //Tracking
    Menu {
        id: boatTrackingSubmenu
        width: rightClickMenuWidth/1.2
        modal: true

        MenuItem {
            contentItem: Label {
                text: (enableTrack ? qsTr("Disable Tracking") : qsTr("Enable Tracking")) + " (T)"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                width: parent.width
            }

            onTriggered: enableTrack = !enableTrack
        }

        MenuItem {
            contentItem: Label {
                text: qsTr("Clear Track") + " (C)"
                enabled: boatTrack.length > 0
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                width: parent.width
            }
            onTriggered: boatTrack = []
        }
    }


    //// Dialog Boxes ////
    Dialog {
        id: centerViewDialog
        modal: false
        closePolicy: Popup.NoAutoClose
        title: qsTr("Center View On Position")
        standardButtons: Dialog.Ok | Dialog.Cancel

        x: (parent.width  - width)  / 2
        y: (parent.height - height) / 2

        background: Rectangle {
            color: "white"
            radius: 12
        }

        Column {
            spacing: 10
            padding: 20

            Label { text: qsTr("Latitude:") }
            TextField {
                id: latInput
                placeholderText: " "
                inputMethodHints: Qt.ImhFormattedNumbersOnly
            }

            Label { text: qsTr("Longitude:") }
            TextField {
                id: lonInput
                placeholderText: " "
                inputMethodHints: Qt.ImhFormattedNumbersOnly
            }

            Label {
                id: errorLabel
                text: ""
                color: "red"
                visible: text !== ""
            }
        }

        onAccepted: {
            var lat = parseFloat(latInput.text)
            var lon = parseFloat(lonInput.text)

            if (isPositionValid(lat,lon)){
                setCenterPosition(lat, lon)
                errorLabel.text = ""
            }
            else{
                errorLabel.text = qsTr("Wrong input")
                centerViewDialog.acc
            }
        }
    }

    Dialog {
        id: dropMarkerDialog
        modal: false
        closePolicy: Popup.NoAutoClose
        title: qsTr("Drop Marker On Position")
        standardButtons: Dialog.Ok | Dialog.Cancel

        x: (parent.width  - width)  / 2
        y: (parent.height - height) / 2

        background: Rectangle {
            color: "white"
            radius: 12
        }

        Column {
            spacing: 10
            padding: 20

            Label { text: qsTr("Latitude:") }
            TextField {
                id: latInputMarker
                placeholderText: " "
                inputMethodHints: Qt.ImhFormattedNumbersOnly
            }

            Label { text: qsTr("Longitude:") }
            TextField {
                id: lonInputMarker
                placeholderText: " "
                inputMethodHints: Qt.ImhFormattedNumbersOnly
            }

            Label {
                id: errorLabelMarker
                text: ""
                color: "red"
                visible: text !== ""
            }
        }

        onAccepted: {
            var lat = parseFloat(latInputMarker.text)
            var lon = parseFloat(lonInputMarker.text)

            if (isPositionValid(lat,lon)){
                addMarkerOnMap(lat, lon)
                errorLabel.text = ""
            }
            else{
                errorLabel.text = qsTr("Wrong input")
                centerViewDialog.acc
            }
        }
    }



    ///////////////
    /// Markers ///
    ///////////////
    //Markers
    Component {
        id: redMarkerImg

        MapQuickItem {
            anchorPoint.x: image.width / 2
            anchorPoint.y: image.height
            coordinate: position

            sourceItem: Image {
                id: image
                width: 30
                height: 30
                source: "qrc:/markers/redMarker"
            }
        }
    }

    //Main boat icon
    Component {
        id: boatMarkerImg

        MapQuickItem {
            anchorPoint.x: image.width / 2
            anchorPoint.y: image.height / 2
            coordinate: position

            sourceItem: Image {
                id: image
                width: 20
                height: 60
                source: "qrc:/boats/boat1"
                opacity: elapsedSec > timeBeforePositionLost ? 0.5 : 1.0
            }
        }
    }



    //////////////
    /// Timers ///
    //////////////
    //Boat Date
    Timer {
        id: updateLastDateTimer
        interval: 1000
        running: true
        repeat: true

        onTriggered: {
            if (timeLastUtcDate === 0)
                return

            elapsedSec = (Date.now() - timeLastUtcDate) / 1000
            if(elapsedSec > timeBeforeGeneralDataLost)
                boatDateReceived = false
        }
    }

    //Boat Time
    Timer {
        id: updateLastTimeTimer
        interval: 1000
        running: true
        repeat: true

        onTriggered: {
            if (timeLastUtcTime === 0)
                return

            elapsedSec = (Date.now() - timeLastUtcTime) / 1000
            if(elapsedSec > timeBeforeGeneralDataLost)
                boatTimeReceived = false
        }
    }

    //Boat Position
    Timer {
        id: updateLastPositionTimer
        interval: 1000
        running: true
        repeat: true

        onTriggered: {
            if (timeLastPosition === 0) {
                return
            }

            elapsedSec = (Date.now() - timeLastPosition) / 1000

            if(elapsedSec <= timeBeforePositionLost)
                if(elapsedSec < 1)
                    textTimerPositionUpdate = qsTr("Position Updated\n < 1s ago")
                else
                    textTimerPositionUpdate = qsTr("Position Updated\n") + Math.ceil(elapsedSec) + qsTr("s ago")
            else{
                boatPositionReceived = false
                textTimerPositionUpdate = qsTr("Position Lost")
                followBoat = false
            }
        }
    }

    //Boat Heading
    Timer {
        id: updateLastHeadingTimer
        interval: 1000
        running: true
        repeat: true

        onTriggered: {
            if (timeLastHeading === 0)
                return

            elapsedSec = (Date.now() - timeLastHeading) / 1000
            if(elapsedSec > timeBeforeGeneralDataLost)
                boatHeadingReceived = false
        }
    }

    //Boat Course
    Timer {
        id: updateLastCourseTimer
        interval: 1000
        running: true
        repeat: true

        onTriggered: {
            if (timeLastCourse === 0)
                return

            elapsedSec = (Date.now() - timeLastCourse) / 1000
            if(elapsedSec > timeBeforeGeneralDataLost)
                boatCourseReceived = false
        }
    }

    //Boat Speed
    Timer {
        id: updateLastSpeedTimer
        interval: 1000
        running: true
        repeat: true

        onTriggered: {
            if (timeLastSpeed === 0)
                return

            elapsedSec = (Date.now() - timeLastSpeed) / 1000
            if(elapsedSec > timeBeforeGeneralDataLost)
                boatSpeedReceived = false
        }
    }

    //Boat Depth
    Timer {
        id: updateLastDepthTimer
        interval: 1000
        running: true
        repeat: true

        onTriggered: {
            if (timeLastDepth === 0)
                return

            elapsedSec = (Date.now() - timeLastDepth) / 1000
            if(elapsedSec > timeBeforeGeneralDataLost)
                boatDepthReceived = false
        }
    }

    //Boat WaterTemp
    Timer {
        id: updateLastWaterTempTimer
        interval: 1000
        running: true
        repeat: true

        onTriggered: {
            if (timeLastWaterTemp === 0)
                return

            elapsedSec = (Date.now() - timeLastWaterTemp) / 1000
            if(elapsedSec > timeBeforeGeneralDataLost)
                boatWaterTemperatureReceived = false
        }
    }

    //Satellites in view
    Timer {
        id: updateLastSatellitesTimer
        interval: 1000
        running: true
        repeat: true

        onTriggered: {
            if (timeLastSatellites === 0)
                return

            elapsedSec = (Date.now() - timeLastSatellites) / 1000
            if(elapsedSec > timeBeforeGeneralDataLost)
                satellitesReceived = false
        }
    }

    //Update boat icon on map
    Timer {
        id: updateMapViewOnBoatTimer
        interval: 1000
        running: true
        repeat: true

        onTriggered: {
            if (followBoat){
                setCenterPositionOnBoat()
            }
        }
    }



    ///////////////////////////////
    /// Data Labels / Left Side ///
    ///////////////////////////////
    Column {
        id: leftSideInfoColumn
        visible: showUI

        anchors.top: parent.top
        anchors.topMargin: labelVerticalMargin * 2
        anchors.left: parent.left
        anchors.leftMargin: labelLateralMargin
        spacing: labelVerticalMargin

        // Map chart/layer
        Label {
            id: mapInfoLabel
            color: labelColor
            width: labelLeftSideWidth
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            padding: labelPadding
            wrapMode: Text.WordWrap
            background: Rectangle {
                color: labelBackgroundColor
                radius: labelBackgroundRadius
            }
            font.pixelSize: 14
            text: qsTr("Chart: OpenTopoMap\nLayer: OpenSeaMap")
        }

        // Zoom level
        Label {
            id: zoomLabel
            color: labelColor
            width: labelLeftSideWidth
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            padding: labelPadding
            background: Rectangle {
                color: labelBackgroundColor
                radius :  labelBackgroundRadius
            }
            font.pixelSize: 14
            text: {
                var percent = (map.zoomLevel - map.minimumZoomLevel) / (map.maximumZoomLevel - map.minimumZoomLevel) * 100
                percent = Math.max(0, Math.min(100, percent))
                return qsTr("Zoom: ") + Math.round(percent) + "%"
            }
        }

        // Map View Mode
        Label {
            id: mapViewModeLabel
            color: labelColor
            width: labelLeftSideWidth
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            padding: labelPadding
            background: Rectangle {
                color: labelBackgroundColor
                radius: labelBackgroundRadius
            }
            font.pixelSize: 14

            text: {
                switch (mapViewMode) {
                    case 0: default: return qsTr("North Up")
                    case 1: return qsTr("Heading Up")
                    case 2: return qsTr("Course Up")
                    case 3: return qsTr("Free View")
                }
            }
        }

        // Cursor position
        Label {
            id: cursorPosition
            color: labelColor
            width: labelLeftSideWidth
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            padding: labelPadding
            background: Rectangle {
                color: labelBackgroundColor
                radius :  labelBackgroundRadius
            }
            font.pixelSize: 14
            text: qsTr("Cursor Position") + "\n" +
                   qsTr("Lat: ") + formatLat(cursorLatitude) + "\n" +
                   qsTr("Lon: ") + formatLon(cursorLongitude)
        }

        // Cursor distance, bearing and ETA from boat
        Label {
            id: distanceBearinFromBoat
            color: labelColor
            width: labelLeftSideWidth
            visible: boatPositionInit
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            padding: labelPadding
            background: Rectangle {
                color: labelBackgroundColor
                radius :  labelBackgroundRadius
            }
            font.pixelSize: 14
            text: qsTr("From Boat") + "\n" +
                  qsTr("Distance: ") + (metersToNauticalMiles(cursorDistanceBoat) > 100
                                 ? metersToNauticalMiles(cursorDistanceBoat).toFixed(0)
                                 : metersToNauticalMiles(cursorDistanceBoat).toFixed(2)) + "NM" + "\n" +
                  qsTr("Bearing: ") + cursorBearingBoat.toFixed(0) + "°" + "\n" +
                  qsTr("ETA: ") + secondsToDHMS(getETA(cursorDistanceBoat, boatSpeed))
        }

        // Last Time Position Update
        Label {
            id: elapsedLabel
            color: labelColor
            width: labelLeftSideWidth
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            padding: labelPadding
            background: Rectangle {
                color: elapsedSec > timeBeforePositionLost ? "indianred" : labelBackgroundColor
                radius :  labelBackgroundRadius
            }
            font.pixelSize: 14
            text: textTimerPositionUpdate
        }
    }



    ////////////////////////////////
    /// Data Labels / Right Side ///
    ////////////////////////////////
    Column {
        id: rightSideInfoColumn
        visible: showUI

        anchors.top: parent.top
        anchors.topMargin: labelVerticalMargin * 2
        anchors.right: parent.right
        anchors.rightMargin: labelLateralMargin
        spacing: labelVerticalMargin

        // Boat Date
        Label {
            id: dateLabel
            color: labelColor
            visible: boatDateReceived
            width: labelRightSideWidth
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            padding: labelPadding
            background: Rectangle {
                color: labelBackgroundColor
                radius :  labelBackgroundRadius
            }
            font.pixelSize: labelFontSize
            text: qsTr("Date: ") + boatDate
        }

        // Boat Time
        Label {
            id: timeLabel
            color: labelColor
            visible: boatTimeReceived
            width: labelRightSideWidth
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            padding: labelPadding
            background: Rectangle {
                color: labelBackgroundColor
                radius :  labelBackgroundRadius
            }
            font.pixelSize: labelFontSize
            text: qsTr("Time: ") + boatTime
        }

        // Satellites in view
        Label {
            id: satellitesInViewLabel
            color: labelColor
            visible: satellitesReceived
            width: labelRightSideWidth
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            padding: labelPadding
            background: Rectangle {
                color: labelBackgroundColor
                radius :  labelBackgroundRadius
            }
            font.pixelSize: labelFontSize
            text: qsTr("Satellites: ") + satellitesInView
        }

        // Boat Position
        Label {
            id: positionLabel
            color: labelColor
            visible: boatPositionInit
            width: labelRightSideWidth
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            padding: labelPadding
            background: Rectangle {
                color: labelBackgroundColor
                radius :  labelBackgroundRadius
            }
            font.pixelSize: labelFontSize
            text: boatPositionReceived ? qsTr("Boat Position\nLat: ") + formatLat(boatLatitude) + "\n"+ qsTr("Lon: ") + formatLon(boatLongitude)
                                       : qsTr("Last Boat Position\nLat: ") + formatLat(boatLatitude) + "\n" + qsTr("Lon: ") + formatLon(boatLongitude)
        }

        // Heading
        Label {
            id: headingLabel
            color: labelColor
            visible: boatHeadingReceived
            width: labelRightSideWidth
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            padding: labelPadding
            background: Rectangle {
                color: labelBackgroundColor
                radius :  labelBackgroundRadius
            }
            font.pixelSize: labelFontSize
            text: qsTr("Heading: ") + boatHeading.toFixed(1) + "°"
        }

        // Course
        Label {
            id: courseLabel
            color: labelColor
            visible: boatCourseReceived
            width: labelRightSideWidth
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            padding: labelPadding
            background: Rectangle {
                color: labelBackgroundColor
                radius :  labelBackgroundRadius
            }
            font.pixelSize: labelFontSize
            text: qsTr("Course: ") + boatCourse.toFixed(1) + "°"
        }

        // Speed
        Label {
            id: speedLabel
            color: labelColor
            visible: boatSpeedReceived
            width: labelRightSideWidth
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            padding: labelPadding
            background: Rectangle {
                color: labelBackgroundColor
                radius :  labelBackgroundRadius
            }
            font.pixelSize: labelFontSize
            text: qsTr("Speed: ") + boatSpeed.toFixed(1) + qsTr("kts")
        }

        // Depth
        Label {
            id: depthLabel
            color: labelColor
            visible: boatDepthReceived
            width: labelRightSideWidth
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            padding: labelPadding
            background: Rectangle {
                color: labelBackgroundColor
                radius :  labelBackgroundRadius
            }
            font.pixelSize: labelFontSize
            text: qsTr("Depth: ") + boatDepth.toFixed(1) + "m"
        }

        // Water Temperature
        Label {
            id: waterTemperatureLabel
            color: labelColor
            visible: boatWaterTemperatureReceived
            width: labelRightSideWidth
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            padding: labelPadding
            background: Rectangle {
                color: labelBackgroundColor
                radius :  labelBackgroundRadius
            }
            font.pixelSize: labelFontSize
            text: qsTr("Water Temp: ") + boatWaterTemperature.toFixed(1) + "°C"
        }
    }

    Canvas {
        id: compassCanvas

        width: 150
        height: 150
        visible: (boatHeadingReceived && showUI)

        anchors.bottom: parent.bottom
        anchors.right: parent.right
        anchors.bottomMargin: labelVerticalMargin
        anchors.rightMargin: labelLateralMargin

        property real heading: boatHeading
        property real course: boatCourse

        onPaint: {
            var ctx = getContext("2d")
            ctx.reset()
            var centerX = width / 2
            var centerY = height / 2
            var radius = Math.min(width, height) / 2 - 10

            // Draw compass circle
            ctx.beginPath()
            ctx.strokeStyle = "black"
            ctx.lineWidth = 2
            ctx.arc(centerX, centerY, radius, 0, 2 * Math.PI)
            ctx.stroke()

            // Draw cardinal directions
            ctx.fillStyle = "black"
            ctx.font = "bold 14px sans-serif"
            ctx.textAlign = "center"
            ctx.textBaseline = "middle"
            ctx.fillText("N",  centerX, centerY - radius + 10);
            ctx.fillText("E",  centerX + radius - 10, centerY);
            ctx.fillText("S",  centerX, centerY + radius - 10);
            ctx.fillText("W",  centerX - radius + 10, centerY);

            // Intercardinal directions
            ctx.font = "10px sans-serif"
            ctx.fillText(qsTr("NE"), centerX + radius * 0.55, centerY - radius * 0.55);
            ctx.fillText(qsTr("SE"), centerX + radius * 0.55, centerY + radius * 0.55);
            ctx.fillText(qsTr("SW"), centerX - radius * 0.55, centerY + radius * 0.55);
            ctx.fillText(qsTr("NW"), centerX - radius * 0.55, centerY - radius * 0.55);

            // Draw heading arrow
            ctx.save()
            ctx.translate(centerX, centerY)
            ctx.rotate((heading - 0) * Math.PI / 180)

            ctx.beginPath()
            ctx.moveTo(0, -radius + 15)
            ctx.lineTo(5, 0)
            ctx.lineTo(-5, 0)
            ctx.closePath()

            ctx.fillStyle = "red"
            ctx.fill()
            ctx.restore()

            // Draw course arrow
            ctx.save()
            ctx.translate(centerX, centerY)
            ctx.rotate((course - 0) * Math.PI / 180)

            ctx.beginPath()
            ctx.moveTo(0, -radius + 20)
            ctx.lineTo(3, 0)
            ctx.lineTo(-3, 0)
            ctx.closePath()

            ctx.fillStyle = "blue"
            ctx.fill()
            ctx.restore()

            // Draw center black circle
            ctx.beginPath()
            ctx.arc(centerX, centerY, 5, 0, 2 * Math.PI)
            ctx.fillStyle = "black"
            ctx.fill()
        }

        // Redraw when heading changes
        Connections {
            target: compassCanvas

            function onHeadingChanged() {
                compassCanvas.requestPaint()
            }

            function onCourseChanged() {
                compassCanvas.requestPaint()
            }
        }
    }



    ///////////////////////////////////////
    /// View-Zoom Actions / Bottom Left ///
    ///////////////////////////////////////
    Column {
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        anchors.leftMargin: labelLateralMargin
        anchors.bottomMargin: labelVerticalMargin
        spacing: labelVerticalMargin

        // Scale bar
        Item {
            id: scaleBar
            width: 140
            height: 28

            property int scaleBarPx: 130
            property real metersPerPixel: {
                var latRad = map.center.latitude * Math.PI / 180
                return metersPerPixelMercatorProjection * Math.cos(latRad) / Math.pow(2, map.zoomLevel)
            }
            property real scaleMeters: metersPerPixel * scaleBarPx

            //Drawing
            Rectangle {
                width: 2
                height: 10
                color: "black"
                anchors.left: parent.left
                anchors.bottom: parent.bottom
            }
            Rectangle {
                width: scaleBar.scaleBarPx
                height: 2
                color: "black"
                anchors.left: parent.left
                anchors.bottom: parent.bottom
            }
            Rectangle {
                width: 2
                height: 10
                color: "black"
                anchors.left: parent.left
                anchors.leftMargin: scaleBar.scaleBarPx - 2
                anchors.bottom: parent.bottom
            }

            // Label
            Text {
                anchors.bottom: parent.top
                anchors.bottomMargin: -25
                anchors.horizontalCenter: parent.horizontalCenter
                font.pixelSize: 12
                color: "black"
                text: {
                    var distMeters = scaleBar.scaleMeters

                    // meters / kilometers formatting
                    var km = distMeters / 1000
                    var distStr =
                        distMeters < 1000
                            ? Math.round(distMeters) + " m"
                            : (km < 10 ? km.toFixed(1) : km.toFixed(0)) + " km"

                    // nautical miles formatting
                    var nm = metersToNauticalMiles(distMeters)
                    var nmStr = (nm < 10 ? nm.toFixed(1) : nm.toFixed(0)) + " NM"

                    return distStr + " / " + nmStr
                }
            }
        }

        // Zoom & Follow
        Row {
            spacing: labelVerticalMargin
            visible: showUI
            Column {
                spacing: labelVerticalMargin / 2

                Row { //Zoom button
                    spacing: labelLateralMargin

                    Button {
                        text: qsTr("Zoom -")
                        width: 60
                        height: 30
                        onClicked: goToZoomLevelMap(map.zoomLevel - 1)
                    }

                    Button {
                        text: qsTr("Zoom +")
                        width: 60
                        height: 30
                        onClicked: goToZoomLevelMap(map.zoomLevel + 1)
                    }
                }

                Slider { //Zoom slider
                    id: zoomSlider
                    from: map.minimumZoomLevel
                    to: map.maximumZoomLevel
                    stepSize: 0.1
                    value: map.zoomLevel
                    width: 130
                    onValueChanged: goToZoomLevelMap(value)
                }
            }

            Button { //Follow Boat
                width: 60
                height: 60
                anchors.verticalCenter: parent.verticalCenter
                enabled: boatPositionReceived
                text: (followBoat ? qsTr("Unfollow Boat") : qsTr("Follow Boat")).replace(" ", "\n")
                onClicked: followBoat = !followBoat
            }
        }
 }



    /////////////////////////////////////////
    /// Update Data From External Signals ///
    /////////////////////////////////////////
    //Update boat UTC time
    function updateBoatTime(time) {
        boatTime = time

        timeLastUtcTime = Date.now()
        boatTimeReceived = true
    }

    //Update boat UTC Date
    function updateBoatDate(date) {
        boatDate = date

        timeLastUtcDate = Date.now()
        boatDateReceived = true
    }

    //Update boat position
    function updateBoatPosition(lat, lon) {
        boatLatitude = lat
        boatLongitude = lon
        currentBoatCoord = QtPositioning.coordinate(lat, lon)

        //Zoom & center on boat first time receiving position
        if(!boatPositionInit){
            goToZoomLevelMap(17)
            setCenterPositionOnBoat()
        }

        timeLastPosition = Date.now()
        boatPositionReceived = true
        boatPositionInit = true

        //Draw new boat position
        updateBoatIconOnMap()

        //Add new position to track line
        drawBoatTrack()
    }

    //Update boat heading
    function updateBoatHeading(head) {
        boatHeading = head

        timeLastHeading= Date.now()
        boatHeadingReceived = true
    }

    //Update boat depth
    function updateBoatDepth(depth) {
        boatDepth = depth

        timeLastDepth = Date.now()
        boatDepthReceived = true
    }

    //Update boat speed
    function updateBoatSpeed(speed) {
        boatSpeed = speed

        timeLastSpeed = Date.now()
        boatSpeedReceived = true
    }

    //Update boat course
    function updateBoatCourse(course) {
        boatCourse = course

        timeLastCourse= Date.now()
        boatCourseReceived = true
    }

    //Update boat water temperature
    function updateBoatWaterTemperature(temp) {
        boatWaterTemperature = temp

        timeLastWaterTemp = Date.now()
        boatWaterTemperatureReceived = true
    }

    //Update number of satellites in view
    function updateSatellitesInView(satellites) {
        satellitesInView = satellites

        timeLastSatellites = Date.now()
        satellitesReceived = true
    }

    /////////////////////////////
    /// Internal Signal-Slots ///
    /////////////////////////////
    Connections {
        //Update boat position relative to cursor
        function onBoatLatitudeChanged() {
            updateBoatCursorCalculations()
        }

        function onBoatLongitudeChanged() {
            updateBoatCursorCalculations()
        }
    }



    //////////////////
    /// Update Map ///
    //////////////////
    //Go To New Position
    function setCenterPosition(targetLat, targetLon) {
        map.center = QtPositioning.coordinate(targetLat, targetLon)
    }

    //Go To Boat Position
    function setCenterPositionOnBoat() {
        map.center = QtPositioning.coordinate(boatLatitude, boatLongitude)
    }

    //Add marker
    function addMarkerOnMap(lat, lon) {
        var item = redMarkerImg.createObject(window, {
            coordinate: QtPositioning.coordinate(lat, lon),
            objectName: "marker"
        });

        map.addMapItem(item)
        userMarkerCount++;
    }

    //Remove all markers from map
    function clearMarkers() {
        for (var i = map.mapItems.length - 1; i >= 0; i--) {
            var item = map.mapItems[i];
            if (item.objectName === "marker") { //Remove all markers with objectName: "marker"
                map.removeMapItem(item);
            }
        }

        userMarkerCount = 0;
    }

    //Increment Map Zoom
    function incrementZoomMap(dz) {
        if (dz > 0)
            map.zoomLevel = Math.min(map.maximumZoomLevel, map.zoomLevel + dz);
        else if (dz < 0)
            map.zoomLevel = Math.max(map.minimumZoomLevel, map.zoomLevel + dz);
    }

    //Go To Zoom Level Map
    function goToZoomLevelMap(zoomLevel) {
        if(zoomLevel > map.maximumZoomLevel)
            zoomLevel = map.maximumZoomLevel
        else if(zoomLevel < map.minimumZoomLevel)
            zoomLevel = map.minimumZoomLevel

        map.zoomLevel = zoomLevel
    }

    //Redraw boat icon
    function updateBoatIconOnMap() {
        // Remove previous boat icon if it exists
        if (boatMarkerRef !== null) {
            map.removeMapItem(boatMarkerRef)
            boatMarkerRef.destroy()
            boatMarkerRef = null
        }

        // Create and store new boat marker
        boatMarkerRef = boatMarkerImg.createObject(window, {
            coordinate: QtPositioning.coordinate(boatLatitude, boatLongitude),
            rotation: boatHeading - mapRotation
        })

        map.addMapItem(boatMarkerRef)
    }

    //Recalculate boat distance/bearing relative to cursor position
    function updateBoatCursorCalculations() {
        cursorDistanceBoat = haversineDistance(boatLatitude, boatLongitude, cursorLatitude, cursorLongitude)
        cursorBearingBoat = calculateBearing(boatLatitude, boatLongitude, cursorLatitude, cursorLongitude)
    }

    //Boat Track
    function drawBoatTrack(){
        //Do not add if track not enabled
        if(!enableTrack)
            return

        //Check if minimum distance between 2 points  before adding
        if (boatTrack.length &&
            haversineDistance(
                boatTrack[boatTrack.length-1].latitude,
                boatTrack[boatTrack.length-1].longitude,
                boatLatitude, boatLongitude
            ) < minimumTrackPointsDistance)
            return

        boatTrack.push(QtPositioning.coordinate(boatLatitude, boatLongitude))

        //Erase first waypoint if array exceeds max size
        if (boatTrack.length > maxTrackPoints)
            boatTrack.shift()
    }



    /////////////////////////
    /// Generic Functions ///
    /////////////////////////
    //Check if position is valid (bool)
    function isPositionValid(lat, lon) {
        if (isNaN(lat) || isNaN(lon))
            return false

        return ((lat >= -90) && (lat <= 90) && (lon >= -180) && (lon <= 180))
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

        return earthRadiusMeters * c
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

    //Get Estimated Time of Arrival (seconds)
    function getETA(distanceFromBoat, speed) {
        if (speed === 0)
            return NaN;

        return distanceFromBoat/knotsToMps(boatSpeed)
    }

    //Calculate end position when adding distance to initial position/heading
    function destinationCoordinate(lat, lon, bearingDeg, distanceMeters) {
        const brng = toRadians(bearingDeg)
        const phi_1 = lat * Math.PI / 180
        const lambda_1 = lon * Math.PI / 180

        const phi_2 = Math.asin(Math.sin(phi_1) * Math.cos(distanceMeters / earthRadiusMeters) +
                             Math.cos(phi_1) * Math.sin(distanceMeters / earthRadiusMeters) * Math.cos(brng))
        const lambda_2 = lambda_1 + Math.atan2(Math.sin(brng) * Math.sin(distanceMeters / earthRadiusMeters) * Math.cos(phi_1),
                                   Math.cos(distanceMeters / earthRadiusMeters) - Math.sin(phi_1) * Math.sin(phi_2))

        return QtPositioning.coordinate(phi_2 * 180 / Math.PI, lambda_2 * 180 / Math.PI)
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
}
