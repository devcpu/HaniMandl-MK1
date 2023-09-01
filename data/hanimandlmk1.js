//window.onload

socket = new WebSocket("ws://%SERVER_IP%/ws");
var isConnected = false;

// Event listener for WebSocket connection open
socket.addEventListener('open', () => {
    console.log('WebSocket connection is open');
    isConnected = true;
  });

  // Event listener for WebSocket connection close
  socket.addEventListener('close', () => {
    console.log('WebSocket connection is closed');
    isConnected = false;
  });

  // Event listener for WebSocket errors
  socket.addEventListener('error', (error) => {
    console.error('WebSocket error:', error);
    isConnected = false;
  });

// Function to send a value via WebSocket
function sendValue(value) {
    if (socket.readyState === WebSocket.OPEN) {
      socket.send(value);
    } else {
      console.error('WebSocket connection is not open');
    }
  }

socket.onmessage = function (evt) {
    var data = JSON.parse(evt.data);
    var glass_count = document.getElementById("glass_count");
    if (glass_count) {
        glass_count.innerHTML = data.glass_count;
    }
    var waagen_gewicht = document.getElementById("waagen_gewicht");
    if (waagen_gewicht) {
        waagen_gewicht.innerHTML = data.waagen_gewicht;
    }
}

  function set_value(key, min, max) {
    var value = parseInt(document.getElementById(key).value, 10);
    console.log("set_value" + key + ":" + value);
    value = isNaN(value) ? 0 : value;
    value >= max ? value = max : '';
    value <=  min ? value = min : '';
    sendValue(key + "=" + value);
  }


  function value_change(key, direction, min, max) {
    var value = parseInt(document.getElementById(key).value, 10);
    console.log("decrease " + key + ":" + value);
    value = isNaN(value) ? 0 : value;
    value >= max ? value = max : '';
    value <=  min ? value = min : '';
    if (direction == 'up') {
        value += 10;
    } else if (direction == 'down') {
        value -= 10;
    }
    sendValue(key + "=" + value);
    document.getElementById(key).value = value;
  }


function sendButton(name) {
    console.log("sendButton :" + name);
    sendValue("button=" + name);
}

function checkReboot() {
    if (window.confirm("eh, really reboot")) {
        x = new XMLHttpRequest();
        //window.setTimeout('window.location = "/rebootinfo"', 0);
        x.open('GET', '/reboot', true);
        x.send();
    }
}

// document.querySelector('input[name=key]:checked').value;

function openSidebar() {
  document.getElementById("mySidebar").style.display = "block";
}

function closeSidebar() {
  document.getElementById("mySidebar").style.display = "none";
}

//     var time = document.getElementById("time");
//     var date = document.getElementById("date");

//     var sensor = document.getElementById("sensor");
//     var temp = document.getElementById("temp");
//     var humidity = document.getElementById("humidity");
//     var pressure = document.getElementById("pressure");

//     if (sensor) {
//         sensor.innerHTML = data.sensor;
//         temp.innerHTML = data.temp + " °C";
//         humidity.innerHTML = data.humidity + " %";
//         pressure.innerHTML = data.pressure + " hPa";
//     }

//     if (data.isValidTime == 1 && time) {
//         time.innerHTML = "Time UTC: " + data.time;
//         date.innerHTML = "Date: " + data.date;
//     }
// };
