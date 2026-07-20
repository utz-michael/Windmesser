#pragma once
#include <Arduino.h>

// =========================================================
//  Dashboard-Seite ( / )
// =========================================================
const char PAGE_INDEX[] PROGMEM = R"HTMLDOC(
<!DOCTYPE html>
<html lang="de">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>Windmesser</title>
<style>
  body{font-family:sans-serif;background:#0f172a;color:#e2e8f0;margin:0;padding:16px;}
  h1{font-size:1.4rem;margin-bottom:4px;}
  .sub{color:#94a3b8;font-size:0.85rem;margin-bottom:16px;}
  .cards{display:flex;flex-wrap:wrap;gap:12px;margin-bottom:20px;}
  .card{background:#1e293b;border-radius:10px;padding:14px 18px;min-width:120px;flex:1;}
  .card .label{font-size:0.75rem;color:#94a3b8;text-transform:uppercase;}
  .card .value{font-size:1.6rem;font-weight:600;margin-top:4px;}
  .maxbox{background:#1e293b;border-radius:10px;padding:14px 18px;margin-bottom:20px;}
  canvas{width:100%;background:#1e293b;border-radius:10px;display:block;}
  a.cfg{color:#60a5fa;font-size:0.85rem;}
  .beaufort{font-size:0.9rem;color:#94a3b8;}
</style>
</head>
<body>
<h1>Windgeschwindigkeit</h1>
<div class="sub" id="status">lade...</div>

<div class="cards">
  <div class="card"><div class="label">m/s</div><div class="value" id="v_ms">--</div></div>
  <div class="card"><div class="label">km/h</div><div class="value" id="v_kmh">--</div></div>
  <div class="card"><div class="label">Knoten</div><div class="value" id="v_kn">--</div></div>
  <div class="card"><div class="label">Beaufort</div><div class="value" id="v_bft">--</div><div class="beaufort" id="v_bft_txt"></div></div>
</div>

<div class="maxbox">
  <div class="label">Maximum (letzte 24h)</div>
  <div class="value" id="v_max">--</div>
  <div class="sub" id="v_max_time"></div>
</div>

<canvas id="chart" height="220"></canvas>
<p><a class="cfg" href="/config">WLAN-Einstellungen</a></p>

<script>
const beaufortText = ["Windstille","leiser Zug","leichte Brise","schwache Brise",
"mäßige Brise","frische Brise","starker Wind","steifer Wind","stürmischer Wind",
"Sturm","schwerer Sturm","orkanartiger Sturm","Orkan"];

function fmtTime(ts){
  const d = new Date(ts*1000);
  return d.toLocaleString('de-DE',{day:'2-digit',month:'2-digit',hour:'2-digit',minute:'2-digit'});
}
function fmtTimeShort(ts){
  const d = new Date(ts*1000);
  return d.toLocaleTimeString('de-DE',{hour:'2-digit',minute:'2-digit'});
}

async function loadCurrent(){
  try{
    const r = await fetch('/api/current');
    const d = await r.json();
    document.getElementById('v_ms').textContent = d.ms.toFixed(1);
    document.getElementById('v_kmh').textContent = d.kmh.toFixed(1);
    document.getElementById('v_kn').textContent = d.kn.toFixed(1);
    document.getElementById('v_bft').textContent = d.bft;
    document.getElementById('v_bft_txt').textContent = beaufortText[d.bft] || '';
    document.getElementById('status').textContent = 'aktualisiert ' + new Date().toLocaleTimeString('de-DE');
  }catch(e){ document.getElementById('status').textContent = 'Verbindungsfehler'; }
}

async function loadMax(){
  try{
    const r = await fetch('/api/max');
    const d = await r.json();
    document.getElementById('v_max').textContent = d.ms.toFixed(1) + ' m/s';
    document.getElementById('v_max_time').textContent = d.t ? fmtTime(d.t) : 'keine Daten';
  }catch(e){}
}

async function loadHistory(){
  try{
    const r = await fetch('/api/history');
    const d = await r.json();
    drawChart(d);
  }catch(e){}
}

function drawChart(points){
  const canvas = document.getElementById('chart');
  const ctx = canvas.getContext('2d');
  const w = canvas.width = canvas.clientWidth;
  const h = canvas.height;
  ctx.clearRect(0,0,w,h);
  if(points.length < 2){
    ctx.fillStyle = '#94a3b8';
    ctx.font = '13px sans-serif';
    ctx.fillText('Noch keine ausreichenden Daten', 10, h/2);
    return;
  }
  const padL = 34, padR = 10, padT = 10, padB = 22;
  const maxV = Math.max(...points.map(p=>p.v), 1);
  const t0 = points[0].t, t1 = points[points.length-1].t;

  // Gitterlinien
  ctx.strokeStyle = '#334155';
  ctx.lineWidth = 1;
  ctx.beginPath();
  for(let i=0;i<=4;i++){
    const y = padT + (h-padT-padB)*i/4;
    ctx.moveTo(padL,y); ctx.lineTo(w-padR,y);
  }
  ctx.stroke();

  // Y-Achsenbeschriftung
  ctx.fillStyle = '#94a3b8';
  ctx.font = '10px sans-serif';
  for(let i=0;i<=4;i++){
    const val = maxV - maxV*i/4;
    const y = padT + (h-padT-padB)*i/4;
    ctx.fillText(val.toFixed(0), 4, y+3);
  }

  // Linie
  ctx.strokeStyle = '#38bdf8';
  ctx.lineWidth = 2;
  ctx.beginPath();
  points.forEach((p,i)=>{
    const x = padL + (w-padL-padR) * (p.t - t0) / ((t1 - t0) || 1);
    const y = padT + (h-padT-padB) * (1 - p.v/maxV);
    if(i===0) ctx.moveTo(x,y); else ctx.lineTo(x,y);
  });
  ctx.stroke();

  // X-Achsenbeschriftung
  ctx.fillStyle = '#94a3b8';
  for(let i=0;i<=4;i++){
    const t = t0 + (t1-t0)*i/4;
    const x = padL + (w-padL-padR)*i/4;
    ctx.fillText(fmtTimeShort(t), x-16, h-6);
  }
}

loadCurrent(); loadMax(); loadHistory();
setInterval(loadCurrent, 5000);
setInterval(loadMax, 30000);
setInterval(loadHistory, 60000);
window.addEventListener('resize', loadHistory);
</script>
</body>
</html>
)HTMLDOC";

// =========================================================
//  Konfigurations-Seite ( /config )
//  Platzhalter %STATUS% %AP_SEL% %STA_SEL% %SSID% %PASSWORD%
//  werden serverseitig per String.replace() ersetzt.
// =========================================================
const char PAGE_CONFIG[] PROGMEM = R"HTMLDOC(
<!DOCTYPE html>
<html lang="de">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>WLAN-Konfiguration</title>
<style>
  body{font-family:sans-serif;background:#0f172a;color:#e2e8f0;margin:0;padding:20px;}
  h1{font-size:1.3rem;}
  label{display:block;margin-top:14px;font-size:0.85rem;color:#94a3b8;}
  input,select{width:100%;padding:8px;margin-top:4px;border-radius:6px;border:1px solid #334155;background:#1e293b;color:#e2e8f0;box-sizing:border-box;}
  button{margin-top:20px;padding:10px 18px;border:none;border-radius:6px;background:#38bdf8;color:#0f172a;font-weight:600;cursor:pointer;}
  a{color:#60a5fa;font-size:0.85rem;}
  .info{background:#1e293b;padding:10px;border-radius:8px;font-size:0.8rem;color:#94a3b8;margin-top:10px;}
</style>
</head>
<body>
<h1>WLAN-Einstellungen</h1>
<div class="info">Aktueller Status: %STATUS%</div>
<form method="POST" action="/config/save">
  <label>Modus</label>
  <select name="mode">
    <option value="AP" %AP_SEL%>Access Point (eigenes WLAN aufspannen)</option>
    <option value="STA" %STA_SEL%>Client (mit bestehendem WLAN verbinden)</option>
  </select>
  <label>SSID (nur für Client-Modus)</label>
  <input type="text" name="ssid" value="%SSID%" maxlength="32">
  <label>Passwort (nur für Client-Modus)</label>
  <input type="password" name="password" value="%PASSWORD%" maxlength="64">
  <button type="submit">Speichern &amp; Neustart</button>
</form>
<p><a href="/">Zurück zum Dashboard</a></p>
</body>
</html>
)HTMLDOC";
