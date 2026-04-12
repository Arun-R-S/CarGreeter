#pragma once

static const char kIndexHtml[] PROGMEM = R"rawliteral(
<!doctype html>
<html>
<head>
  <meta charset="utf-8"/>
  <meta name="viewport" content="width=device-width,initial-scale=1"/>
  <title>CarGreeter</title>
  <style>
    body{font-family:system-ui,Arial;margin:20px;max-width:720px}
    button,input{font-size:16px;padding:10px;margin:6px 0}
    .row{display:flex;gap:10px;flex-wrap:wrap;align-items:center}
    .card{border:1px solid #ddd;border-radius:10px;padding:14px}
    .muted{color:#666}
  </style>
</head>
<body>
  <h1>CarGreeter</h1>
  <div class="card">
    <div class="row">
      <button onclick="fetch('/play').then(r=>r.text()).then(alert)">Play</button>
      <a href="/logview">View Logs</a>
    </div>
    <div class="row">
      <label>Delay (seconds)</label>
      <input id="delay" type="number" min="0" value="5"/>
      <button onclick="setDelay()">Set</button>
    </div>
    <div class="row">
      <form id="up" method="post" action="/upload" enctype="multipart/form-data">
        <input type="file" name="file" accept=".wav"/>
        <button type="submit">Upload WAV</button>
      </form>
    </div>
    <p class="muted">All endpoints require Basic Auth.</p>
  </div>
  <script>
    function setDelay(){
      const v = document.getElementById('delay').value || '0';
      fetch('/setDelay?value=' + encodeURIComponent(v)).then(r=>r.text()).then(alert);
    }
  </script>
</body>
</html>
)rawliteral";

static const char kLogViewHtml[] PROGMEM = R"rawliteral(
<!doctype html>
<html>
<head>
  <meta charset="utf-8"/>
  <meta name="viewport" content="width=device-width,initial-scale=1"/>
  <title>CarGreeter Logs</title>
  <style>
    body{font-family:system-ui,Arial;margin:20px;max-width:720px}
    pre{border:1px solid #ddd;border-radius:10px;padding:14px;white-space:pre-wrap}
    button{font-size:16px;padding:10px}
  </style>
</head>
<body>
  <h1>Logs</h1>
  <button onclick="load()">Refresh</button>
  <pre id="logs">Loading...</pre>
  <script>
    async function load(){
      const t = await fetch('/logs').then(r=>r.text());
      document.getElementById('logs').textContent = t || '(empty)';
    }
    setInterval(load, 1500);
    load();
  </script>
</body>
</html>
)rawliteral";

