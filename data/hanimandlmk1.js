//window.onload

socket = new WebSocket("ws://%SERVER_IP%/ws");

var reload_timeout = 10;
function checkReload() {
    //FIXME - test only
    var glass_count = document.getElementById("glass_count");
    if (glass_count) {
        glass_count.innerHTML = reload_timeout;
    }

    reload_timeout -= 1;
    if (reload_timeout <= 0) {
        alert("Keine Verbindung mehr zm Hanimandl!");
        location.reload();
    }
}

setInterval(checkReload, 1000);
// socket.onopen = function (evt) {
// }


// Event listener for WebSocket connection open
// socket.addEventListener('open', () => {
//     console.log('WebSocket connection is open');
//     isConnected = true;
//   });

//   // Event listener for WebSocket connection close
//   socket.addEventListener('close', () => {
//     console.log('WebSocket connection is closed');
//     isConnected = false;
//   });

//   // Event listener for WebSocket errors
//   socket.addEventListener('error', (error) => {
//     console.error('WebSocket error:', error);
//     isConnected = false;
//   });

// Function to send a value via WebSocket
function sendValue(value) {
    if (socket.readyState === WebSocket.OPEN) {
      socket.send(value);
    } else {
      console.error('WebSocket connection is not open');
    }
  }

socket.onmessage = function (evt) {
    if (reload_timeout < 10) {
        reload_timeout += 2;
    }

    var data = JSON.parse(evt.data);

    //FIXME - active for release
    // var glass_count = document.getElementById("glass_count");
    // if (glass_count) {
    //     glass_count.innerHTML = data.glass_count;
    // }

    var waagen_gewicht = document.getElementById("waagen_gewicht");
    if (waagen_gewicht) {
        waagen_gewicht.innerHTML = data.waagen_gewicht;
    }

    var run_modus = document.getElementById("run_modus");
    if (run_modus) {
        run_modus.innerHTML = data.run_modus;
    }
};

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

function showHide(show, hide) {
    console.log("show:" + show);
    console.log("show:" + hide);
    var element2show = document.getElementById(show);
    var element2hide = document.getElementById(hide);
    // console.log("element2show:" + element2show.innerHTML);
    // console.log("element2hide" + element2hide.innerHTML);
    // element2show.style.visibility = 'visible';
    // element2hide.style.visibility = 'collapse';
    if (element2show) {
        element2show.style.display = 'block';
    }
    if (element2hide) {
        element2hide.style.display = 'none';
    }
}

// function show(key, show_element_id) {
//     var name_key = "input[name=" + key + "]:checked";
//     var element2show = document.getElementById(show_element_id);
//     // 'input[name=key]:checked'
//     console.log("querySelector=" + name_key);
//     // console.log("ElementById" + element2show.innerHTML);
//     // var keyValue = document.querySelector(name_key).value;
//     // console.log("checked=" + keyValue);
//         // console.log("show" + key + ":" + keyValue);
//     if (document.querySelector(name_key).value) {
//         element2show.style.display = 'block';
//     } else {
//         element2show.style.display = 'none';
//     }

// }

function show(key, show_element_id) {
    console.log(key);
    console.log(show_element_id);
    var checkbox = document.getElementById(key);
    var element2show = document.getElementById(show_element_id);
    console.log(element2show.innerHTML);
    if (checkbox.checked) {
        element2show.style.display = 'block';
    } else {
        element2show.style.display = 'none';
    }

}

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
