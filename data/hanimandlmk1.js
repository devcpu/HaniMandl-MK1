//window.onload


var ws = new WebSocket("ws://%SERVER_IP%/ws");
// ws.onopen = function() {
//     window.alert("Connected");
//  };
ws.onmessage = function (evt) {

    var data = JSON.parse(evt.data);

    // window.alert(data);

    var time = document.getElementById("time");
    var date = document.getElementById("date");

    var gps_lat = document.getElementById("gps_lat");
    var gps_lng = document.getElementById("gps_lng");
    var gps_direction = document.getElementById("gps_direction");
    var gps_speed = document.getElementById("gps_speed");
    var gps_sat = document.getElementById("gps_sat");
    var gps_hdop = document.getElementById("gps_hdop");

    var sensor = document.getElementById("sensor");
    var temp = document.getElementById("temp");
    var humidity = document.getElementById("humidity");
    var pressure = document.getElementById("pressure");

    if (sensor) {
        sensor.innerHTML = data.sensor;
        temp.innerHTML = data.temp + " °C";
        humidity.innerHTML = data.humidity + " %";
        pressure.innerHTML = data.pressure + " hPa";
    }

    if (data.isValidTime == 1 && time) {
        time.innerHTML = "Time UTC: " + data.time;
        date.innerHTML = "Date: " + data.date;
    }


    if (data.isValidGPS == 1 && gps_lat) {
        gps_lat.innerHTML = data.lat;
        gps_lng.innerHTML = data.lng;
        gps_course.innerHTML = data.course;
        gps_speed.innerHTML = data.speed;
        gps_sat.innerHTML = data.sat;
        gps_hdop.innerHTML = data.hdop;

    }

};






function checkReboot() {
    if (window.confirm("eh, really reboot")) {
        x = new XMLHttpRequest();
        //window.setTimeout('window.location = "/rebootinfo"', 0);
        x.open('GET', '/reboot', true);
        x.send();

    }

}
