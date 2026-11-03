#pragma once
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include "state.h"

// The board runs its own Wi-Fi network and answers every web address with the
// control page, so phones pop it up like a hotel sign-in page.

WebServer server(80);
DNSServer dns;
bool webRunning = false;
char wifiName[32];

const char CONTROL_PAGE[] PROGMEM = R"page(<!doctype html>
<html><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>Solar Square</title>
<style>
body{font-family:system-ui,sans-serif;background:#111;color:#eee;margin:0 auto;padding:16px;max-width:420px}
h1{font-size:22px;margin:4px 0 12px}
h2{font-size:14px;color:#999;margin:22px 0 8px;font-weight:600;text-transform:uppercase;letter-spacing:.05em}
button{background:#222;color:#eee;border:1px solid #333;border-radius:10px;padding:12px 6px;font-size:15px;text-transform:capitalize}
button.on{background:#f5a000;color:#111;border-color:#f5a000}
.modes{display:grid;grid-template-columns:repeat(3,1fr);gap:8px}
input[type=range]{width:100%}
.colors{display:flex;flex-wrap:wrap;gap:8px;align-items:center;margin-bottom:10px}
.sw{width:34px;height:34px;border-radius:50%;border:2px solid #444;padding:0}
.sw.on{border-color:#fff;transform:scale(1.15)}
input[type=color]{width:38px;height:38px;border:none;background:none;padding:0}
.grid{display:grid;grid-template-columns:repeat(8,1fr);gap:3px;background:#000;padding:6px;border-radius:10px;touch-action:none}
.grid div{aspect-ratio:1;border-radius:4px;border:1px solid #222}
.row{display:flex;gap:8px;margin-top:10px}.row button{flex:1}
</style></head><body>
<h1>Solar Square</h1>
<h2>Pattern</h2><div class="modes" id="modes"></div>
<h2>Brightness</h2><input type="range" id="bright" min="1" max="255">
<h2>Draw</h2>
<div class="colors" id="colors"><input type="color" id="custom" value="#ff00ff"></div>
<div class="grid" id="grid"></div>
<div class="row"><button id="clear">Clear</button><button id="fill">Fill</button></div>
<script>
const $ = id => document.getElementById(id);
const COLORS = ['ff0000','ff7a00','ffd000','00e040','00c8ff','0040ff','9000ff','ff3c9a','ffffff','000000'];
let color = COLORS[0], pixels = Array(64).fill('000000'), sendTimer, brightTimer, dragging = false;

function render(s) {
  $('modes').innerHTML = '';
  s.modes.forEach((name, i) => {
    const b = document.createElement('button');
    b.textContent = name;
    if (i == s.mode) b.className = 'on';
    b.onclick = () => post('/api/mode?i=' + i);
    $('modes').append(b);
  });
  if (document.activeElement != $('bright')) $('bright').value = s.brightness;
}
function post(url, body) {
  return fetch(url, {method: 'POST', body}).then(r => r.json()).then(render);
}
function drawGrid() {
  [...$('grid').children].forEach((cell, i) => cell.style.background = '#' + pixels[i]);
}
function sendCanvas() {
  clearTimeout(sendTimer);
  sendTimer = setTimeout(() => post('/api/canvas', pixels.join('')), 120);
}
function paint(e) {
  const cell = document.elementFromPoint(e.clientX, e.clientY);
  if (!cell || cell.dataset.i === undefined || pixels[cell.dataset.i] == color) return;
  pixels[cell.dataset.i] = color;
  drawGrid();
  sendCanvas();
}
function pick(hex, el) {
  color = hex;
  document.querySelectorAll('.sw').forEach(s => s.classList.remove('on'));
  if (el) el.classList.add('on');
}

COLORS.forEach((hex, i) => {
  const s = document.createElement('button');
  s.className = 'sw' + (i ? '' : ' on');
  s.style.background = '#' + hex;
  s.title = hex == '000000' ? 'Eraser' : '#' + hex;
  s.onclick = () => pick(hex, s);
  $('colors').insertBefore(s, $('custom'));
});
$('custom').oninput = e => pick(e.target.value.slice(1), null);

for (let i = 0; i < 64; i++) {
  const cell = document.createElement('div');
  cell.dataset.i = i;
  $('grid').append(cell);
}
$('grid').onpointerdown = e => { dragging = true; paint(e); };
$('grid').onpointermove = e => { if (dragging) paint(e); };
window.onpointerup = () => dragging = false;

$('clear').onclick = () => { pixels.fill('000000'); drawGrid(); sendCanvas(); };
$('fill').onclick = () => { pixels.fill(color); drawGrid(); sendCanvas(); };
$('bright').oninput = e => {
  clearTimeout(brightTimer);
  brightTimer = setTimeout(() => post('/api/brightness?value=' + e.target.value), 100);
};

fetch('/api/state').then(r => r.json()).then(s => {
  render(s);
  pixels = s.canvas.match(/.{6}/g);
  drawGrid();
});
setInterval(() => fetch('/api/state').then(r => r.json()).then(render), 3000);
</script></body></html>)page";

String stateJson() {
  String json = "{\"mode\":";
  json += mode;
  json += ",\"brightness\":";
  json += brightness;
  json += ",\"modes\":[";
  for (uint8_t i = 0; i < MODE_COUNT; i++) {
    if (i) json += ',';
    json += '"';
    json += MODES[i].name;
    json += '"';
  }
  json += "],\"canvas\":\"";
  char hex[7];
  for (uint16_t i = 0; i < NUM_LEDS; i++) {
    snprintf(hex, sizeof(hex), "%02x%02x%02x", canvas[i].r, canvas[i].g, canvas[i].b);
    json += hex;
  }
  json += "\"}";
  return json;
}

void sendState() {
  server.send(200, "application/json", stateJson());
}

static uint8_t hexByte(const char* p) {
  char pair[3] = {p[0], p[1], 0};
  return strtoul(pair, nullptr, 16);
}

void handleCanvas() {
  const String& body = server.arg("plain");
  if (body.length() < NUM_LEDS * 6) {
    server.send(400, "text/plain", "expected 64 colors as rrggbb");
    return;
  }
  const char* p = body.c_str();
  for (uint16_t i = 0; i < NUM_LEDS; i++, p += 6) {
    canvas[i] = CRGB(hexByte(p), hexByte(p + 2), hexByte(p + 4));
  }
  canvasChanged();
  sendState();
}

void startWeb() {
  if (!WIFI_ENABLED) return;

  WiFi.mode(WIFI_AP);
  uint8_t mac[6];
  WiFi.softAPmacAddress(mac);
  snprintf(wifiName, sizeof(wifiName), "%s-%02X%02X", WIFI_NAME_PREFIX, mac[4], mac[5]);
  WiFi.softAP(wifiName, WIFI_PASSWORD);
  // Many SuperMini boards have a weak antenna match and drop Wi-Fi at full power.
  WiFi.setTxPower(WIFI_POWER_8_5dBm);

  IPAddress ip = WiFi.softAPIP();
  dns.start(53, "*", ip);

  server.on("/", HTTP_GET, []() { server.send_P(200, "text/html", CONTROL_PAGE); });
  server.on("/api/state", HTTP_GET, sendState);
  server.on("/api/mode", HTTP_POST, []() {
    setMode(server.arg("i").toInt());
    sendState();
  });
  server.on("/api/brightness", HTTP_POST, []() {
    setBrightness(constrain(server.arg("value").toInt(), 1, 255));
    sendState();
  });
  server.on("/api/canvas", HTTP_POST, handleCanvas);
  server.onNotFound([ip]() {
    server.sendHeader("Location", String("http://") + ip.toString() + "/", true);
    server.send(302, "text/plain", "");
  });
  server.begin();
  webRunning = true;

  Serial.printf("Wi-Fi: join \"%s\" (password \"%s\"), then open http://%s\n",
                wifiName, WIFI_PASSWORD, ip.toString().c_str());
}

void handleWeb() {
  if (!webRunning) return;
  dns.processNextRequest();
  server.handleClient();
}
