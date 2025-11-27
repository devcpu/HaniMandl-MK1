/* eslint-env browser */
/* Project: Honey Filling Machine MK I Web UI
 * Linting via ESLint (.eslintrc.json). Console usage is allowed for
 * diagnostics.
 */

// window.onload

let socket = null;
let reconnectAttempts = 0;
let lastMessageTs = Date.now();
let lastHeartbeatTs = Date.now();
let lastWeightTs = Date.now();
let connectionWarned = false;
let connectionDead = false;    // Track if we already showed dead warning
const RELOAD_DEAD_MS = 60000;  // 60s ohne Nachricht -> reload
const WARN_AFTER_MS = 20000;   // 20s Warnung
const STALE_WEIGHT_MS = 5000;  // nach 5s ohne Update als stale markieren
const CLIENT_PING_INTERVAL_MS = 25000;  // eigenes leichtes Keepalive

function markButtons(enabled) {
  const ids = [
    'startbutton', 'finebutton', 'stopbutton', 'hand', 'auto', 'alarmstopbutton'
  ];  // relevante Haupt-Buttons
  ids.forEach(id => {
    const el = document.getElementById(id);
    if (el) el.disabled = !enabled;
  });
}

function openSocket() {
  const url = 'ws://%SERVER_IP%/ws';
  try {
    socket && socket.close();
  } catch (e) {
  }
  socket = new WebSocket(url);
  const attempt = ++reconnectAttempts;
  console.log('WS connecting attempt #' + attempt);
  socket.onopen = () => {
    console.log('WS connected');
    reconnectAttempts = 0;
    connectionWarned = false;
    connectionDead = false;  // Reset dead flag on reconnect
    markButtons(true);
    // Remove any dead connection overlay if present
    const overlay = document.getElementById('connection-overlay');
    if (overlay) overlay.style.display = 'none';
  };
  socket.onclose = (ev) => {
    console.warn('WS closed code=' + ev.code + ' reason=' + (ev.reason || ''));
    markButtons(false);
    scheduleReconnect();
  };
  socket.onerror = (err) => {
    console.error('WS error', err);
    try {
      socket.close();
    } catch (e) {
    }
  };
  socket.onmessage = handleMessage;
}

function scheduleReconnect() {
  const base = 500;    // 0.5s
  const maxD = 10000;  // 10s
  const delay = Math.min(maxD, base * Math.pow(2, reconnectAttempts));
  console.log(
      'Reconnecting in ' + delay + 'ms (attempt=' + reconnectAttempts + ')');
  setTimeout(() => {
    openSocket();
  }, delay);
}

// Show non-blocking connection lost overlay
function showConnectionOverlay() {
  let overlay = document.getElementById('connection-overlay');
  if (!overlay) {
    overlay = document.createElement('div');
    overlay.id = 'connection-overlay';
    overlay.style.cssText = `
      position: fixed; top: 0; left: 0; right: 0; bottom: 0;
      background: rgba(0,0,0,0.8); z-index: 9999;
      display: flex; align-items: center; justify-content: center;
      flex-direction: column; color: white; font-size: 1.5em;
    `;
    overlay.innerHTML = `
      <div style="text-align: center; padding: 20px; background: #d32f2f; border-radius: 10px; max-width: 400px;">
        <h2>⚠️ Verbindung verloren</h2>
        <p>Keine Verbindung zur Honey Filling Machine</p>
        <button onclick="location.reload()" style="margin: 10px; padding: 10px 20px; font-size: 1em; cursor: pointer;">
          Seite neu laden
        </button>
        <button onclick="document.getElementById('connection-overlay').style.display='none'"
                style="margin: 10px; padding: 10px 20px; font-size: 1em; cursor: pointer;">
          Ignorieren
        </button>
      </div>
    `;
    document.body.appendChild(overlay);
  }
  overlay.style.display = 'flex';
}

// Client Ping (falls sonst keine Aktivität) – verhindert Idle Drops auf einigen
// Setups
setInterval(() => {
  if (!socket || socket.readyState !== WebSocket.OPEN) return;
  const now = Date.now();
  if (now - lastMessageTs > CLIENT_PING_INTERVAL_MS / 2) {
    try {
      socket.send('ping=1');
    } catch (e) {
      console.warn('ping send failed', e);
    }
  }
}, CLIENT_PING_INTERVAL_MS);

setInterval(() => {
  const now = Date.now();
  const since = now - lastMessageTs;
  if (since > WARN_AFTER_MS && !connectionWarned) {
    console.warn('Verbindung verzögert (' + since + 'ms)');
    connectionWarned = true;
  }
  if (since > RELOAD_DEAD_MS && !connectionDead) {
    connectionDead = true;
    console.error('Verbindung tot - zeige Overlay');
    // Show overlay instead of blocking alert
    showConnectionOverlay();
  }
  // Heartbeat Latenz-Anzeige
  const hbEl = document.getElementById('hb_latency');
  if (hbEl) {
    const hbDelta = (now - lastHeartbeatTs) / 1000;
    hbEl.innerHTML = hbDelta.toFixed(1);
  }
  // Stale-Markierung für Gewichte
  const wEl = document.getElementById('waagen_gewicht');
  const hEl = document.getElementById('honey_gewicht');
  const stale = (now - lastWeightTs) > STALE_WEIGHT_MS;
  [wEl, hEl].forEach(el => {
    if (el) {
      if (stale)
        el.classList.add('stale');
      else
        el.classList.remove('stale');
    }
  });
}, 1000);
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
    console.log('Value sent:', value);
  } else {
    console.error('WebSocket connection is not open');
  }
}

function setText(id, value) {
  var el = document.getElementById(id);
  if (el !== null && el !== undefined) {
    el.innerHTML = value;
  }
}

let initialSnapshotReceived = false;

// Helper to consistently render integer gram values (round to nearest g)
function asInt(value) {
  if (value === undefined || value === null) return 0;
  const n = Number(value);
  if (!isFinite(n)) return value;  // leave weird payloads untouched
  return Math.round(n);
}

function handleMessage(evt) {
  lastMessageTs = Date.now();
  let msg;
  try {
    msg = JSON.parse(evt.data);
  } catch (e) {
    console.error('Bad JSON', evt.data);
    return;
  }

  // Snapshot full state
  if (msg.type === 'snapshot') {
    initialSnapshotReceived = true;
    setText('waagen_gewicht', asInt(msg.weight_current));
    setText('honey_gewicht', asInt(msg.weight_honey || 0));
    setText('run_modus', msg.run_modus);
    setText('glass_count', msg.glass_count);
    // Optional: weitere Felder (servo, wifi)
    return;
  }

  // Incremental events (compact t codes)
  if (msg.t) {
    switch (msg.t) {
      case 'w':
        setText('waagen_gewicht', asInt(msg.v));
        lastWeightTs = Date.now();
        if (msg.h !== undefined) {
          setText('honey_gewicht', asInt(msg.h));
          lastWeightTs = Date.now();
        }
        break;
      case 'fs':
        setText('filling_status', msg.v);
        break;
      case 'rm':
        setText('run_modus', msg.v);
        break;
      case 'ws':
        if (msg.connected) {
          setText('wifi_state', msg.ap ? 'AP' : 'STA');
        } else {
          setText('wifi_state', msg.ap ? 'AP' : 'OFF');
        }
        break;
      case 'ntp':
        // ntp synced indicator optional
        break;
      case 'hb':
        lastHeartbeatTs = Date.now();
        break;
      case 'gc':
        setText('glass_count', msg.v);
        break;
    }
    return;
  }

  // Fallback für alte/komplette Nachrichten (Kompatibilität)
  if (msg.waagen_gewicht !== undefined) {
    setText('waagen_gewicht', asInt(msg.waagen_gewicht));
  }
  if (msg.honey_gewicht !== undefined) {
    setText('honey_gewicht', asInt(msg.honey_gewicht));
  }
  if (msg.run_modus !== undefined) {
    setText('run_modus', msg.run_modus);
  }
}

// initial connect
openSocket();

function set_value(key, min, max) {
  var value = parseInt(document.getElementById(key).value, 10);
  console.log('set_value' + key + ':' + value);
  value = isNaN(value) ? 0 : value;
  value >= max ? value = max : '';
  value <= min ? value = min : '';
  sendValue(key + '=' + value);
}


function value_change(key, direction, min, max) {
  var value = parseInt(document.getElementById(key).value, 10);
  console.log('change' + key + ':' + value);
  value = isNaN(value) ? 0 : value;
  value >= max ? value = max : '';
  value <= min ? value = min : '';
  if (direction == 'up') {
    value += 10;
  } else if (direction == 'down') {
    value -= 10;
  }
  sendValue(key + '=' + value);
  document.getElementById(key).value = value;
}


function sendButton(name) {
  sendValue('button=' + name);
  console.log('sendButton :' + name);
}

function sendButton2(name, field_name) {
  console.log('sendButton2 :' + name + ' ' + field_name);
  var x = document.getElementById(field_name).value;
  console.log('x:' + x);
  var value = parseInt(x, 10);
  console.log('value:' + value);
  var request_value = 'button=' + name + '&' + field_name + '=' + value;
  sendValue(request_value);
  console.log('sendButton2 :' + request_value);
}


function checkReboot() {
  if (window.confirm('eh, really reboot')) {
    const x = new XMLHttpRequest();
    // window.setTimeout('window.location = "/rebootinfo"', 0);
    x.open('GET', '/reboot', true);
    x.send();
  }
}

function showHide(show, hide) {
  console.log('show:' + show);
  console.log('show:' + hide);
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
  document.getElementById('mySidebar').style.display = 'block';
}

function closeSidebar() {
  document.getElementById('mySidebar').style.display = 'none';
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
