#pragma once

static const char kIndexHtml[] PROGMEM = R"rawliteral(
<!doctype html>
<html>
<head>
  <meta charset="utf-8"/>
  <meta name="viewport" content="width=device-width,initial-scale=1"/>
  <title>CarGreeter</title>
  <style>
    :root{--bg:#0b1020;--card:#121a33;--text:#e9ecf5;--muted:#aab3d3;--accent:#6ea8ff;--border:#243056;--danger:#ff6b6b}
    *{box-sizing:border-box}
    body{font-family:system-ui,-apple-system,Segoe UI,Roboto,Arial;margin:0;background:linear-gradient(180deg,#070a14, #0b1020);color:var(--text)}
    header{padding:18px 16px;border-bottom:1px solid rgba(255,255,255,.06);position:sticky;top:0;background:rgba(11,16,32,.85);backdrop-filter:blur(10px)}
    h1{margin:0;font-size:20px}
    main{padding:16px;max-width:980px;margin:0 auto;display:grid;gap:12px}
    .grid{display:grid;gap:12px}
    @media(min-width:880px){.grid{grid-template-columns:1fr 1fr}}
    .card{background:rgba(18,26,51,.92);border:1px solid rgba(255,255,255,.08);border-radius:14px;padding:14px}
    .title{display:flex;align-items:center;justify-content:space-between;margin-bottom:10px}
    .title h2{margin:0;font-size:16px}
    .muted{color:var(--muted);font-size:13px}
    .row{display:flex;gap:10px;flex-wrap:wrap;align-items:center}
    label{font-size:13px;color:var(--muted)}
    input,select,textarea{width:100%;background:#0b1020;border:1px solid rgba(255,255,255,.12);color:var(--text);border-radius:12px;padding:10px 12px;font-size:15px;outline:none}
    input:focus,select:focus,textarea:focus{border-color:rgba(110,168,255,.7);box-shadow:0 0 0 3px rgba(110,168,255,.18)}
    textarea{min-height:120px;font-family:ui-monospace,SFMono-Regular,Menlo,Consolas,monospace;font-size:13px}
    .two{display:grid;gap:10px}
    @media(min-width:520px){.two{grid-template-columns:1fr 1fr}}
    button{background:linear-gradient(180deg,#6ea8ff,#4b82ff);border:0;color:#04102b;border-radius:12px;padding:10px 12px;font-size:15px;font-weight:700;cursor:pointer}
    .pill{display:inline-flex;align-items:center;gap:8px;padding:8px 10px;border:1px solid rgba(255,255,255,.12);border-radius:999px;font-size:13px;color:var(--muted)}
    .list{margin-top:10px;display:grid;gap:8px}
    .net{display:flex;justify-content:space-between;gap:10px;border:1px solid rgba(255,255,255,.10);border-radius:12px;padding:10px 12px;background:rgba(11,16,32,.55)}
    .net b{color:var(--text)}
    button.secondary{
      background:transparent;color:var(--text);
      border:1px solid rgba(255, 255, 255, .14)
    }
    button.danger{
      background:linear-gradient(180deg, #ff6b6b, #ff4d4d);
      color: #2b0505
    }
    button.success {
    background: linear-gradient(180deg, #6bffab, #4dffa0);
    color: #000000;
    }
    button.primary {
    background: linear-gradient(180deg, #22719f, #0b508d);
    color: #000000;
    }
    button.warning {
    background: linear-gradient(180deg, #ffcc00, #ff9900);
    color: #000000;
    }
    button.info {
    background: linear-gradient(180deg, #66ccff, #3399ff);
    color: #000000;
    }
    button:hover {
      opacity: 0.85;
    }
  </style>
</head>
<body>
  <header>
    <div class="row" style="justify-content:space-between;align-items:center">
      <h1>CarGreeter</h1>
      <a href="/logview" style="color:var(--accent);text-decoration:none;font-weight:700">Logs</a>
    </div>
    <div class="row" style="margin-top:10px;gap:8px">
      <span class="pill" id="netMode">Mode: …</span>
      <span class="pill" id="netIp">IP: …</span>
    </div>
  </header>
  <main>
    <div class="grid">
      <section class="card">
        <div class="title"><h2>Playback</h2><span class="muted">JQ6500 indexed tracks</span></div>
        <div class="row">
          <button class="success" onclick="apiGet('/play')">Play now</button>
          <button class="warning" onclick="refresh()">Refresh</button>
        </div>
        <div class="two" style="margin-top:10px">
          <div>
            <label>Preloaded welcome track</label>
            <select id="preloaded">
              <option value="1">1 - Tata.mp3</option>
              <option value="2">2 - Mahindra.mp3</option>
              <option value="3">3 - Toyota.mp3</option>
              <option value="4">4 - ToyotaInnova.mp3</option>
              <option value="5">5 - Hyundai.mp3</option>
              <option value="6">6 - HyundaiAirlines.mp3</option>
              <option value="7">7 - InnovaAirlines.mp3</option>
              <option value="8">8 - Chevrolet.mp3</option>
              <option value="9">9 - ChevroletSpark.mp3</option>
            </select>
          </div>
          <div>
            <label>Custom track index (0 disables)</label>
            <input id="custom" type="number" min="0" max="9999" value="0"/>
          </div>
        </div>
        <div class="two" style="margin-top:10px">
          <div>
            <label>Volume (0–30)</label>
            <input id="volume" type="number" min="0" max="30" value="20"/>
          </div>
          <div style="display:flex;align-items:flex-end">
            <button style="width:100%" onclick="saveAudio()">Save audio settings</button>
          </div>
        </div>
        <div class="muted" id="audioHint" style="margin-top:8px">Effective track: …</div>
      </section>

      <section class="card">
        <div class="title"><h2>Boot Delay</h2><span class="muted">seconds</span></div>
        <div class="two">
          <div>
            <label>Delay</label>
            <input id="delay" type="number" min="0" max="3600" value="5"/>
          </div>
          <div class="secondary" style="display:flex;align-items:flex-end">
            <button style="width:100%" onclick="setDelay()">Save delay</button>
          </div>
        </div>
        <p class="muted" style="margin-top:10px">Tip: set 0 for immediate playback.</p>
      </section>
    </div>

    <div class="grid">
      <section class="card">
        <div class="title"><h2>Hotspot (AP)</h2><span class="muted">used when WiFi not configured</span></div>
        <div class="two">
          <div>
            <label>Hotspot name (SSID)</label>
            <input id="apSsid" maxlength="32" placeholder="CarGreeter"/>
          </div>
          <div>
            <label>Hotspot password (min 8)</label>
            <input id="apPassword" type="password" minlength="8" maxlength="64" placeholder="********"/>
          </div>
        </div>
        <div class="row" style="margin-top:10px">
          <button class="success" onclick="saveHotspot()">Save hotspot</button>
        </div>
        <p class="muted">Password is not shown; enter a new one to change it.</p>
      </section>

      <section class="card">
        <div class="title"><h2>WiFi (STA)</h2><span class="muted">connect using saved credentials</span></div>
        <div class="muted" id="wifiStatus">Status: …</div>
        <div class="row">
          <button class="warning" onclick="scanWifi()">Scan</button>
          <button class="danger" onclick="forgetWifi()">Forget WiFi</button>
        </div>
        <div class="list" id="wifiList"></div>
        <div class="two" style="margin-top:10px">
          <div>
            <label>SSID</label>
            <input id="wifiSsid" maxlength="32" placeholder="Select from list or type"/>
          </div>
          <div>
            <label>Password</label>
            <input id="wifiPassword" type="password" maxlength="64" placeholder="WiFi password"/>
          </div>
        </div>
        <div class="row" style="margin-top:10px">
          <button class="success" onclick="connectWifi()">Save & restart</button>
        </div>
        <p class="muted">If WiFi isn’t configured, device will always start the hotspot.</p>
      </section>
    </div>

    <section class="card">
      <div class="title"><h2>System</h2><span class="muted">maintenance</span></div>
      <div class="row">
        <button class="warning" onclick="restart()">Restart</button>
        <button class="danger" onclick="factoryReset()">Factory reset</button>
        <button class="info" onclick="downloadBackup()">Backup</button>
      </div>
      <div style="margin-top:12px">
        <label>Restore (paste backup text and restore)</label>
        <textarea id="restoreText" placeholder="delaySeconds=5&#10;volume=20&#10;..."></textarea>
        <div class="row" style="margin-top:10px">
          <button class="primary" onclick="restore()">Restore & restart</button>
        </div>
      </div>
      <p class="muted">All endpoints require Basic Auth.</p>
    </section>
  </main>
  <script>
    function toast(msg){ alert(msg); }
    async function apiGet(url){ const t = await fetch(url).then(r=>r.text()); toast(t); }
    async function apiPost(url, body){
      const r = await fetch(url, {method:'POST', headers:{'Content-Type':'application/x-www-form-urlencoded'}, body});
      const t = await r.text();
      if(!r.ok){ throw new Error(t || ('HTTP ' + r.status)); }
      return t;
    }
    async function refresh(){
      // Sync device time from browser clock (works for both STA and AP mode).
      try{
        await apiPost('/api/time/sync', 'epochMs=' + encodeURIComponent(String(Date.now())));
      }catch(e){}
      const s = await fetch('/api/settings').then(r=>r.json());
      document.getElementById('delay').value = s.delaySeconds;
      document.getElementById('volume').value = s.volume;
      document.getElementById('preloaded').value = String(s.preloadedIndex);
      document.getElementById('custom').value = String(s.customIndex);
      document.getElementById('apSsid').value = s.apSsid || '';
      document.getElementById('netMode').textContent = 'Mode: ' + (s.mode || '…');
      document.getElementById('netIp').textContent = 'IP: ' + (s.ip || '…');
      document.getElementById('audioHint').textContent = 'Effective track: ' + s.effectiveIndex;
      const st = s.connected ? ('Connected to ' + (s.connectedSsid||'') + ' (' + s.rssi + ' dBm)') : 'Not connected';
      document.getElementById('wifiStatus').textContent = 'Status: ' + st;
    }
    async function setDelay(){
      const v = document.getElementById('delay').value || '0';
      toast(await fetch('/setDelay?value=' + encodeURIComponent(v)).then(r=>r.text()));
      refresh();
    }
    async function saveAudio(){
      const pre = document.getElementById('preloaded').value || '1';
      const custom = document.getElementById('custom').value || '0';
      const vol = document.getElementById('volume').value || '20';
      await apiPost('/api/audio', 'preloaded=' + encodeURIComponent(pre) + '&custom=' + encodeURIComponent(custom) + '&volume=' + encodeURIComponent(vol));
      toast('Audio settings saved');
      refresh();
    }
    async function saveHotspot(){
      const ssid = document.getElementById('apSsid').value || '';
      const pass = document.getElementById('apPassword').value || '';
      toast(await apiPost('/api/hotspot', 'ssid=' + encodeURIComponent(ssid) + '&password=' + encodeURIComponent(pass)));
    }
    function rssiBars(rssi){
      if(rssi > -55) return 'Excellent';
      if(rssi > -67) return 'Good';
      if(rssi > -80) return 'Ok';
      return 'Weak';
    }
    async function scanWifi(){
      const list = document.getElementById('wifiList');
      list.innerHTML = '<div class="muted">Scanning…</div>';
      const nets = await fetch('/api/wifi/scan').then(r=>r.json());
      if(!nets || !nets.length){ list.innerHTML = '<div class="muted">No networks yet. Tap Scan again.</div>'; return; }
      list.innerHTML = '';
      nets.forEach(n=>{
        const el = document.createElement('div');
        el.className='net';
        el.innerHTML = '<div><b>' + (n.ssid||'(hidden)') + '</b><div class="muted">' + rssiBars(n.rssi) + ' (' + n.rssi + ' dBm)</div></div><div class="muted">' + (n.enc? 'Locked' : 'Open') + '</div>';
        el.onclick = ()=>{ document.getElementById('wifiSsid').value = n.ssid||''; };
        list.appendChild(el);
      });
    }
    async function connectWifi(){
      const ssid = document.getElementById('wifiSsid').value || '';
      const pass = document.getElementById('wifiPassword').value || '';
      toast(await apiPost('/api/wifi/connect', 'ssid=' + encodeURIComponent(ssid) + '&password=' + encodeURIComponent(pass)));
    }
    async function forgetWifi(){
      toast(await apiPost('/api/wifi/forget', ''));
    }
    async function restart(){
      toast(await apiPost('/api/restart', ''));
    }
    async function factoryReset(){
      if(!confirm('Factory reset settings and restart?')) return;
      toast(await apiPost('/api/factory_reset', 'confirm=YES'));
    }
    async function downloadBackup(){
      const txt = await fetch('/api/backup').then(r=>r.text());
      const blob = new Blob([txt], {type:'text/plain'});
      const a = document.createElement('a');
      a.href = URL.createObjectURL(blob);
      a.download = 'car_greeter_backup.txt';
      a.click();
      URL.revokeObjectURL(a.href);
    }
    async function restore(){
      const txt = document.getElementById('restoreText').value || '';
      toast(await fetch('/api/restore', {method:'POST', headers:{'Content-Type':'text/plain'}, body:txt}).then(r=>r.text()));
    }
    refresh();
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
    :root{--bg:#0b1020;--card:#121a33;--text:#e9ecf5;--muted:#aab3d3;--accent:#6ea8ff}
    *{box-sizing:border-box}
    body{font-family:system-ui,-apple-system,Segoe UI,Roboto,Arial;margin:0;background:linear-gradient(180deg,#070a14, #0b1020);color:var(--text)}
    header{padding:18px 16px;border-bottom:1px solid rgba(255,255,255,.06);position:sticky;top:0;background:rgba(11,16,32,.85);backdrop-filter:blur(10px)}
    h1{margin:0;font-size:20px}
    main{padding:16px;max-width:980px;margin:0 auto;display:grid;gap:12px}
    .card{background:rgba(18,26,51,.92);border:1px solid rgba(255,255,255,.08);border-radius:14px;padding:14px}
    .row{display:flex;gap:10px;flex-wrap:wrap;align-items:center}
    .muted{color:var(--muted);font-size:13px}
    button{background:linear-gradient(180deg,#6ea8ff,#4b82ff);border:0;color:#04102b;border-radius:12px;padding:10px 12px;font-size:15px;font-weight:700;cursor:pointer}
    button.secondary{background:transparent;color:var(--text);border:1px solid rgba(255,255,255,.14)}
    pre{margin:0;border:1px solid rgba(255,255,255,.10);border-radius:12px;padding:12px;white-space:pre-wrap;background:rgba(11,16,32,.55);min-height:320px}
    a{color:var(--accent);text-decoration:none;font-weight:700}
    label{display:flex;align-items:center;gap:8px;font-size:13px;color:var(--muted)}
  </style>
</head>
<body>
  <header>
    <div class="row" style="justify-content:space-between;align-items:center">
      <h1>Logs</h1>
      <a href="/">Back</a>
    </div>
    <div class="row" style="margin-top:10px">
      <button class="secondary" onclick="load()">Refresh</button>
      <button class="secondary" onclick="copyLogs()">Copy</button>
      <label><input id="auto" type="checkbox" checked/> Auto refresh</label>
      <label><input id="scroll" type="checkbox" checked/> Auto scroll</label>
      <span class="muted" id="status">…</span>
    </div>
  </header>
  <main>
    <section class="card">
      <div class="muted" style="margin-bottom:10px">Timestamps are device time (IST). It starts at Jan 01 2026 11:00 AM IST on boot and syncs when WiFi/NTP or a browser opens this page.</div>
      <pre id="logs">Loading...</pre>
    </section>
  </main>
  <script>
    async function apiPost(url, body){
      const r = await fetch(url, {method:'POST', headers:{'Content-Type':'application/x-www-form-urlencoded'}, body});
      const t = await r.text();
      if(!r.ok){ throw new Error(t || ('HTTP ' + r.status)); }
      return t;
    }
    async function syncTime(){
      try{ await apiPost('/api/time/sync', 'epochMs=' + encodeURIComponent(String(Date.now()))); }catch(e){}
    }
    async function load(){
      const st = document.getElementById('status');
      try{
        st.textContent = 'Loading…';
        const t = await fetch('/logs').then(r=>r.text());
        const el = document.getElementById('logs');
        el.textContent = t || '(empty)';
        if(document.getElementById('scroll').checked){
          el.scrollTop = el.scrollHeight;
        }
        st.textContent = 'Updated';
      }catch(e){
        st.textContent = 'Error: ' + (e && e.message ? e.message : e);
      }
    }
    function copyLogs(){
      const t = document.getElementById('logs').textContent || '';
      if(navigator.clipboard && navigator.clipboard.writeText){
        navigator.clipboard.writeText(t);
      }else{
        alert('Clipboard not available');
      }
    }
    setInterval(()=>{ if(document.getElementById('auto').checked) load(); }, 1500);
    setInterval(syncTime, 180000);
    syncTime();
    load();
  </script>
</body>
</html>
)rawliteral";
