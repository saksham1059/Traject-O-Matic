#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <Wire.h>

// WiFi Configuration
const char* ssid = "Ping Pong in Space";
const char* password = ""; // No password

// Server Configuration
const byte DNS_PORT = 53;
IPAddress apIP(192, 168, 4, 1);
IPAddress netMsk(255, 255, 255, 0);

ESP8266WebServer server(80);
DNSServer dnsServer;

// Ultrasonic (HC-SR04) pins (3.3V-only wiring)
const uint8_t TRIG_PIN = 14; // D5 on NodeMCU
const uint8_t ECHO_PIN = 12; // D6 on NodeMCU

// I2C (MPU6050)
const uint8_t I2C_SDA_PIN = 4;  // D2
const uint8_t I2C_SCL_PIN = 5;  // D1
const uint8_t MPU6050_ADDR = 0x68; // AD0=GND -> 0x68, AD0=VCC -> 0x69

// LDR (analog) on A0 pin with voltage divider
const uint8_t LDR_PIN = A0; // Analog pin A0

// HTML Content - Satellite Dashboard (stored in flash memory)
const char htmlContent[] PROGMEM = "<!DOCTYPE html><html lang='en'><head><meta charset='UTF-8'><meta name='viewport' content='width=device-width, initial-scale=1.0'><title>Satellite Dashboard</title><style>*{margin:0;padding:0;box-sizing:border-box}body{font-family:Arial,sans-serif;background:linear-gradient(135deg,#0c1445 0%,#1a2980 50%,#26d0ce 100%);min-height:100vh;color:#fff;overflow-x:hidden}.stars{position:fixed;top:0;left:0;width:100%;height:100%;background:transparent url('data:image/svg+xml,<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 100 100\"><circle cx=\"20\" cy=\"20\" r=\"1\" fill=\"white\" opacity=\"0.8\"/><circle cx=\"80\" cy=\"40\" r=\"0.5\" fill=\"white\" opacity=\"0.6\"/><circle cx=\"40\" cy=\"80\" r=\"1.5\" fill=\"white\" opacity=\"0.9\"/><circle cx=\"90\" cy=\"10\" r=\"0.8\" fill=\"white\" opacity=\"0.7\"/><circle cx=\"10\" cy=\"90\" r=\"1.2\" fill=\"white\" opacity=\"0.8\"/></svg>') repeat;animation:twinkle 4s ease-in-out infinite;z-index:-2}.twinkling{position:fixed;top:0;left:0;width:100%;height:100%;animation:twinkle 3s ease-in-out infinite reverse;z-index:-1}@keyframes twinkle{0%,100%{opacity:0.3}50%{opacity:1}}.popup-overlay{position:fixed;top:0;left:0;width:100%;height:100%;background:rgba(0,0,0,0.8);display:flex;justify-content:center;align-items:center;z-index:1000;backdrop-filter:blur(5px)}.popup-content{background:linear-gradient(135deg,#1a2980 0%,#26d0ce 100%);border-radius:20px;padding:30px;max-width:500px;width:90%;box-shadow:0 20px 40px rgba(0,0,0,0.3);border:2px solid rgba(255,255,255,0.1);animation:popupSlideIn 0.5s ease-out}@keyframes popupSlideIn{from{transform:translateY(-50px);opacity:0}to{transform:translateY(0);opacity:1}}.popup-header h2{text-align:center;margin-bottom:20px;font-size:1.8rem}.popup-body p{text-align:center;margin-bottom:15px;color:#e0e0e0;font-size:1.1rem}.sensor-status{margin:20px 0}.sensor-item{display:flex;justify-content:space-between;align-items:center;padding:10px 0;border-bottom:1px solid rgba(255,255,255,0.1)}.sensor-name{font-weight:500}.status.disconnected{color:#ff6b6b;font-weight:600}.status.online{color:#4caf50;font-weight:600}.continue-btn{width:100%;padding:15px;background:linear-gradient(135deg,#667eea 0%,#764ba2 100%);border:none;border-radius:10px;color:white;font-size:1.1rem;font-weight:600;cursor:pointer;transition:all 0.3s ease;margin-top:20px}.continue-btn:hover{transform:translateY(-2px);box-shadow:0 10px 20px rgba(0,0,0,0.2)}.dashboard{min-height:100vh;padding:20px;transition:opacity 0.5s ease}.dashboard.hidden{display:none}.dashboard-header{display:flex;justify-content:space-between;align-items:center;margin-bottom:30px;padding:20px;background:rgba(255,255,255,0.1);border-radius:15px;backdrop-filter:blur(10px);border:1px solid rgba(255,255,255,0.2)}.dashboard-header h1{font-size:2.5rem;background:linear-gradient(135deg,#667eea 0%,#764ba2 100%);-webkit-background-clip:text;-webkit-text-fill-color:transparent;background-clip:text}.status-indicator{display:flex;align-items:center;gap:10px}.status-dot{width:12px;height:12px;border-radius:50%;animation:pulse 2s infinite}.status-dot.demo{background:#ffa726}.status-dot.online{background:#4caf50}@keyframes pulse{0%,100%{opacity:1}50%{opacity:0.5}}.status-text{font-weight:600}.dashboard-content{display:grid;grid-template-columns:repeat(auto-fit,minmax(350px,1fr));gap:25px;max-width:1400px;margin:0 auto}.sensor-card{background:rgba(255,255,255,0.1);border-radius:20px;padding:25px;backdrop-filter:blur(10px);border:1px solid rgba(255,255,255,0.2);transition:all 0.3s ease;position:relative;overflow:hidden}.sensor-card:hover{transform:translateY(-5px);box-shadow:0 15px 30px rgba(0,0,0,0.3)}.card-header{display:flex;justify-content:space-between;align-items:center;margin-bottom:20px}.card-header h3{font-size:1.4rem}.card-status{display:flex;align-items:center;gap:8px;font-size:0.9rem;color:#e0e0e0}.temperature-display{text-align:center;margin-bottom:20px}.temperature-value{font-size:3rem;font-weight:bold;color:#ff6b6b;text-shadow:0 0 10px rgba(255,107,107,0.5)}.temperature-unit{font-size:1.5rem;color:#e0e0e0;margin-left:5px}.temperature-gauge{margin-top:20px}.gauge-background{width:100%;height:20px;background:rgba(255,255,255,0.1);border-radius:10px;overflow:hidden;position:relative}.gauge-fill{height:100%;background:linear-gradient(90deg,#4caf50,#ff9800,#f44336);border-radius:10px;transition:width 0.5s ease;width:35%}.gauge-labels{display:flex;justify-content:space-between;margin-top:8px;font-size:0.9rem;color:#e0e0e0}.axis-values{display:flex;justify-content:space-around;margin-bottom:10px}.axis-box{text-align:center}.axis-label{display:block;font-size:0.9rem;color:#e0e0e0;margin-bottom:5px}.axis-value{font-size:1.5rem;font-weight:bold;color:#64b5f6;text-shadow:0 0 10px rgba(100,181,246,0.5)}.graph-container{width:100%;height:200px;background:rgba(0,0,0,0.2);border-radius:10px;padding:10px;margin-top:15px}canvas{width:100%;height:100%;border-radius:8px}.light-indicator{text-align:center;padding:20px 0}.light-bulb{width:80px;height:80px;border-radius:50%;background:radial-gradient(circle,#ffeb3b 0%,#ffc107 70%,#ff9800 100%);margin:0 auto 20px;position:relative;box-shadow:0 0 30px rgba(255,235,59,0.6);animation:lightPulse 2s infinite}.light-bulb.dark{background:radial-gradient(circle,#424242 0%,#212121 70%,#000000 100%);box-shadow:0 0 10px rgba(0,0,0,0.3)}@keyframes lightPulse{0%,100%{transform:scale(1)}50%{transform:scale(1.05)}}.light-text{font-size:1.2rem;font-weight:600}.radar-display{display:flex;align-items:center;gap:20px}.radar-circle{width:120px;height:120px;border-radius:50%;background:rgba(0,0,0,0.3);position:relative;border:2px solid #4caf50;overflow:hidden}.radar-sweep{position:absolute;top:50%;left:50%;width:50%;height:2px;background:linear-gradient(90deg,transparent,#4caf50);transform-origin:left center;animation:radarSweep 3s linear infinite}@keyframes radarSweep{from{transform:rotate(0deg)}to{transform:rotate(360deg)}}.radar-target{position:absolute;width:8px;height:8px;background:#ff5722;border-radius:50%;top:30%;left:60%;animation:targetBlink 1s infinite}@keyframes targetBlink{0%,100%{opacity:1}50%{opacity:0.3}}.radar-info{flex:1}.radar-distance,.radar-status{display:flex;justify-content:space-between;margin-bottom:10px;padding:8px 0;border-bottom:1px solid rgba(255,255,255,0.1)}.label{color:#e0e0e0;font-weight:500}.value{color:#4caf50;font-weight:600}.uptime-display{text-align:center;padding:20px 0}.uptime-value{font-size:2.5rem;font-weight:bold;color:#4caf50;text-shadow:0 0 10px rgba(76,175,80,0.5);font-family:'Courier New',monospace;margin-bottom:10px}.uptime-label{font-size:0.9rem;color:#e0e0e0}.modes-actions{display:flex;gap:10px;flex-wrap:wrap}.mode-btn{padding:10px 14px;border-radius:8px;border:1px solid rgba(255,255,255,0.2);background:rgba(255,255,255,0.1);color:#fff;cursor:pointer}.mode-btn:hover{background:rgba(255,255,255,0.2)}.mode-panel{margin-top:12px;padding:12px;border-radius:10px;background:rgba(0,0,0,0.2);border:1px solid rgba(255,255,255,0.1)}.mode-row{display:flex;gap:16px;align-items:center;flex-wrap:wrap}.pill{display:inline-block;padding:6px 10px;border-radius:999px;background:rgba(255,255,255,0.12);border:1px solid rgba(255,255,255,0.15)}@media (max-width:768px){.dashboard-header{flex-direction:column;gap:15px;text-align:center}.dashboard-header h1{font-size:2rem}.dashboard-content{grid-template-columns:1fr;gap:20px}.sensor-card{padding:20px}.temperature-value{font-size:2.5rem}.uptime-value{font-size:2rem}.radar-display{flex-direction:column;text-align:center}.radar-circle{width:100px;height:100px}}@media (max-width:480px){.dashboard{padding:10px}.popup-content{padding:20px;margin:10px}.dashboard-header h1{font-size:1.5rem}.temperature-value{font-size:2rem}.axis-values{flex-direction:column;gap:10px}}</style></head><body><div id='connectionPopup' class='popup-overlay'><div class='popup-content'><div class='popup-header'><h2>⚠️ Sensor Connection Alert</h2></div><div class='popup-body'><p>Sensors are not connected to the satellite system.</p><p>Demo mode will be activated with simulated data.</p><div class='sensor-status'><div class='sensor-item'><span class='sensor-name'>Temperature Sensor:</span><span class='status online'>✅ Connected</span></div><div class='sensor-item'><span class='sensor-name'>Accelerometer:</span><span class='status online'>✅ Connected</span></div><div class='sensor-item'><span class='sensor-name'>Gyroscope:</span><span class='status online'>✅ Connected</span></div><div class='sensor-item'><span class='sensor-name'>Light Sensor:</span><span class='status online'>✅ Connected</span></div><div class='sensor-item'><span class='sensor-name'>Ultrasonic Radar:</span><span class='status online'>✅ Connected</span></div></div></div><div class='popup-footer'><button id='continueBtn' class='continue-btn'>Continue with Demo Data</button></div></div></div><div id='dashboard' class='dashboard hidden'><div class='stars'></div><div class='twinkling'></div><header class='dashboard-header'><h1>🛰️ Satellite Control Dashboard</h1><div class='status-indicator'><span class='status-dot online'></span><span class='status-text'>Online</span></div></header><main class='dashboard-content'><div class='sensor-card temperature-card'><div class='card-header'><h3>🌡️ Temperature Sensor</h3><div class='card-status'><span class='status-dot demo'></span><span>Demo Data</span></div></div><div class='card-body'><div class='temperature-display'><span id='temperatureValue' class='temperature-value'>32.5</span><span class='temperature-unit'>°C</span></div><div class='temperature-gauge'><div class='gauge-background'><div id='temperatureGauge' class='gauge-fill'></div></div><div class='gauge-labels'><span>30°C</span><span>37°C</span></div></div></div></div><div class='sensor-card accelerometer-card'><div class='card-header'><h3>🧭 Accelerometer</h3><div class='card-status'><span class='status-dot online'></span><span>Live</span></div></div><div class='card-body'><div class='axis-values'><div class='axis-box'><span class='axis-label'>AX</span><span id='accelX' class='axis-value'>0.0</span></div><div class='axis-box'><span class='axis-label'>AY</span><span id='accelY' class='axis-value'>0.0</span></div><div class='axis-box'><span class='axis-label'>AZ</span><span id='accelZ' class='axis-value'>0.0</span></div></div><div class='graph-container'><canvas id='accelGraph' width='400' height='200'></canvas></div></div></div><div class='sensor-card gyroscope-card'><div class='card-header'><h3>🔄 Gyroscope</h3><div class='card-status'><span class='status-dot online'></span><span>Live</span></div></div><div class='card-body'><div class='axis-values'><div class='axis-box'><span class='axis-label'>GX</span><span id='gyroX' class='axis-value'>0.0</span></div><div class='axis-box'><span class='axis-label'>GY</span><span id='gyroY' class='axis-value'>0.0</span></div><div class='axis-box'><span class='axis-label'>GZ</span><span id='gyroZ' class='axis-value'>0.0</span></div></div><div class='graph-container'><canvas id='gyroGraph' width='400' height='200'></canvas></div></div></div><div class='sensor-card light-card'><div class='card-header'><h3>💡 Light Intensity</h3><div class='card-status'><span class='status-dot online'></span><span>Live</span></div></div><div class='card-body'><div class='light-indicator'><div id='lightStatus' class='light-bulb'></div><div class='light-text'><span id='lightText'>Light: 0%</span></div></div></div></div><div class='sensor-card radar-card'><div class='card-header'><h3>📡 Ultrasonic Radar</h3><div class='card-status'><span class='status-dot online'></span><span>Live</span></div></div><div class='card-body'><div class='radar-display'><div class='radar-circle'><div class='radar-sweep'></div><div class='radar-target' id='radarTarget'></div></div><div class='radar-info'><div class='radar-distance'><span class='label'>Distance:</span><span id='radarDistance' class='value'>--</span></div><div class='radar-status'><span class='label'>Status:</span><span id='radarStatus' class='value'>Waiting</span></div></div></div></div></div><div class='sensor-card modes-card'><div class='card-header'><h3>🧪 Advanced Modes</h3><div class='card-status'><span class='status-dot demo'></span><span>On Hold</span></div></div><div class='card-body'><div style='text-align:center;padding:20px;color:#e0e0e0;'><p style='margin-bottom:15px;font-size:1.1rem;'>🚀 Projectile Calculator</p><p style='margin-bottom:15px;font-size:1.1rem;'>⚖️ Gravity Calculator</p><p style='margin-bottom:15px;font-size:1.1rem;'>🌀 Spin Meter</p><p style='margin-bottom:15px;font-size:1.1rem;'>📐 Level Tool</p><p style='margin-bottom:15px;font-size:1.1rem;'>🪂 Freefall Detection</p><p style='margin-bottom:15px;font-size:1.1rem;'>🧭 Compass</p><div style='background:rgba(255,255,255,0.1);padding:15px;border-radius:10px;margin-top:20px;'><p style='margin:0;font-size:0.9rem;opacity:0.8;'>Advanced modes are currently on hold.</p><p style='margin:5px 0 0 0;font-size:0.9rem;opacity:0.8;'>Focusing on core sensor functionality.</p></div></div></div></div><div class='sensor-card uptime-card'><div class='card-header'><h3>⏱️ System Uptime</h3><div class='card-status'><span class='status-dot online'></span><span>Online</span></div></div><div class='card-body'><div class='uptime-display'><div class='uptime-value' id='uptimeValue'>00:00:00</div><div class='uptime-label'>Hours:Minutes:Seconds</div></div></div></div></main></div><script>let espUptimeBase=0;let accelData={x:[],y:[],z:[]},gyroData={x:[],y:[],z:[]},accelCtx,gyroCtx;const connectionPopup=document.getElementById('connectionPopup'),continueBtn=document.getElementById('continueBtn'),dashboard=document.getElementById('dashboard'),accelX=document.getElementById('accelX'),accelY=document.getElementById('accelY'),accelZ=document.getElementById('accelZ'),gX=document.getElementById('gyroX'),gY=document.getElementById('gyroY'),gZ=document.getElementById('gyroZ'),accelGraph=document.getElementById('accelGraph'),gyroGraph=document.getElementById('gyroGraph'),lightStatus=document.getElementById('lightStatus'),lightText=document.getElementById('lightText'),radarDistance=document.getElementById('radarDistance'),radarStatus=document.getElementById('radarStatus'),radarTarget=document.getElementById('radarTarget'),uptimeValue=document.getElementById('uptimeValue');async function getEspUptime(){try{const r=await fetch('/uptime');if(r.ok){const t=await r.text();espUptimeBase=parseInt(t,10)||0;}}catch(e){espUptimeBase=0;}}function init(){continueBtn.addEventListener('click',startDashboard);getEspUptime();}function setupGraph(canvas){const ctx=canvas.getContext('2d');const rect=canvas.getBoundingClientRect();canvas.width=rect.width;canvas.height=rect.height;return ctx;}function ensureBuffers(obj){while(obj.x.length<100){obj.x.push(0);obj.y.push(0);obj.z.push(0);}}function startDashboard(){connectionPopup.style.display='none';dashboard.classList.remove('hidden');accelCtx=setupGraph(accelGraph);gyroCtx=setupGraph(gyroGraph);ensureBuffers(accelData);ensureBuffers(gyroData);startTemperatureSimulation();startLightPolling();startRadarPolling();startUptimeCounter();startIMUPolling();}function startTemperatureSimulation(){setInterval(()=>{const temp=30+Math.random()*7;const displayTemp=temp.toFixed(1);document.getElementById('temperatureValue').textContent=displayTemp;const gaugePercentage=((temp-30)/7)*100;document.getElementById('temperatureGauge').style.width=`${gaugePercentage}%`;},2000);}function startIMUPolling(){setInterval(async()=>{try{const r=await fetch('/imu');if(!r.ok)throw new Error('imu');const d=await r.json();accelX.textContent=d.ax.toFixed(2);accelY.textContent=d.ay.toFixed(2);accelZ.textContent=d.az.toFixed(2);gX.textContent=d.gx.toFixed(1);gY.textContent=d.gy.toFixed(1);gZ.textContent=d.gz.toFixed(1);accelData.x.push(d.ax);accelData.y.push(d.ay);accelData.z.push(d.az);gyroData.x.push(d.gx);gyroData.y.push(d.gy);gyroData.z.push(d.gz);['x','y','z'].forEach(k=>{if(accelData[k].length>100)accelData[k].shift();if(gyroData[k].length>100)gyroData.z.shift();});updateGraph(accelCtx,accelGraph,accelData,-2,2,['#ff6b6b','#4caf50','#64b5f6']);updateGraph(gyroCtx,gyroGraph,gyroData,-250,250,['#ff6b6b','#4caf50','#64b5f6']);}catch(e){}},100);}function updateGraph(ctx,canvas,data,minVal,maxVal,colors){const w=canvas.width,h=canvas.height;ctx.clearRect(0,0,w,h);ctx.strokeStyle='rgba(255,255,255,0.1)';ctx.lineWidth=1;for(let i=0;i<=10;i++){const x=(w/10)*i;ctx.beginPath();ctx.moveTo(x,0);ctx.lineTo(x,h);ctx.stroke();}for(let i=0;i<=5;i++){const y=(h/5)*i;ctx.beginPath();ctx.moveTo(0,y);ctx.lineTo(w,y);ctx.stroke();}const range=maxVal-minVal;['x','y','z'].forEach((k,idx)=>{ctx.strokeStyle=colors[idx];ctx.lineWidth=2;ctx.beginPath();const arr=data[k];const step=w/Math.max(1,(arr.length-1));for(let i=0;i<arr.length;i++){const x=i*step;const norm=(arr[i]-minVal)/range;const y=h-(norm*h);if(i===0)ctx.moveTo(x,y);else ctx.lineTo(x,y);}ctx.stroke();});}function startLightPolling(){setInterval(async()=>{try{const r=await fetch('/light');if(!r.ok)throw 0;const t=await r.text();const lightValue=parseInt(t.trim(),10);const invertedValue=1023-lightValue;const brightness=Math.min(100,Math.max(0,invertedValue/1023*100));if(brightness>10){lightStatus.classList.remove('dark');lightStatus.style.filter=`brightness(${0.3+brightness/100*0.7})`;lightText.textContent=`Light: ${invertedValue}`;}else{lightStatus.classList.add('dark');lightStatus.style.filter='brightness(0.1)';lightText.textContent=`Dark: ${invertedValue}`;}}catch(e){lightStatus.classList.add('dark');lightStatus.style.filter='brightness(0.1)';lightText.textContent='Offline';}},250);}let sweepAngle=0;function startRadarPolling(){setInterval(async()=>{try{const r=await fetch('/distance');if(!r.ok)throw new Error('http');const t=await r.text();const cm=parseInt(t,10);if(cm>0){radarDistance.textContent=`${(cm/100).toFixed(2)}m`;radarStatus.textContent='Active';const center=60;const minR=10,maxR=50;const rr=minR+Math.min(maxR-minR,Math.max(0,cm/300*(maxR-minR)));sweepAngle=(sweepAngle+15)%360;const rad=sweepAngle*Math.PI/180;const tx=center+Math.cos(rad)*rr;const ty=center+Math.sin(rad)*rr;radarTarget.style.left=`${tx}px`;radarTarget.style.top=`${ty}px`;}else{radarDistance.textContent='--';radarStatus.textContent='No Echo';}}catch(e){radarStatus.textContent='Offline';}},500);}function startUptimeCounter(){let base=espUptimeBase;let started=Date.now();setInterval(()=>{let elapsed=Math.floor(base+(Date.now()-started)/1000);const hours=Math.floor(elapsed/3600);const minutes=Math.floor((elapsed%3600)/60);const seconds=elapsed%60;const uptimeString=`${hours.toString().padStart(2,'0')}:${minutes.toString().padStart(2,'0')}:${seconds.toString().padStart(2,'0')}`;uptimeValue.textContent=uptimeString;},1000);}document.addEventListener('DOMContentLoaded',init);</script></body></html>";

// --------- Sensors: Ultrasonic ---------
// Measure distance in cm; returns -1 if no echo within timeout
long measureDistanceCm() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  unsigned long duration = pulseIn(ECHO_PIN, HIGH, 25000UL); // ~4m timeout
  if (duration == 0) return -1;
  float cm = duration / 58.0f;
  if (cm > 500.0f) return -1; // cap unrealistic
  return (long)cm;
}

// --------- Sensors: MPU6050 ---------
bool mpuWrite(uint8_t reg, uint8_t val) {
  Wire.beginTransmission(MPU6050_ADDR);
  Wire.write(reg);
  Wire.write(val);
  return Wire.endTransmission() == 0;
}

bool mpuReadBytes(uint8_t reg, uint8_t* buf, uint8_t len) {
  Wire.beginTransmission(MPU6050_ADDR);
  Wire.write(reg);
  if (Wire.endTransmission(false) != 0) return false; // repeated start
  if (Wire.requestFrom(MPU6050_ADDR, len) != len) return false;
  for (uint8_t i = 0; i < len; i++) buf[i] = Wire.read();
  return true;
}

bool mpuInit() {
  // Wake up device: PWR_MGMT_1 (0x6B) = 0
  if (!mpuWrite(0x6B, 0x00)) return false;
  // Set accelerometer range ±2g: ACCEL_CONFIG (0x1C) = 0x00
  if (!mpuWrite(0x1C, 0x00)) return false;
  // Set gyro range ±250°/s: GYRO_CONFIG (0x1B) = 0x00
  if (!mpuWrite(0x1B, 0x00)) return false;
  return true;
}

bool mpuRead(float& ax, float& ay, float& az, float& gx, float& gy, float& gz) {
  uint8_t buf[14];
  if (!mpuReadBytes(0x3B, buf, 14)) return false; // ACCEL_XOUT_H...
  int16_t rawAx = (int16_t)((buf[0] << 8) | buf[1]);
  int16_t rawAy = (int16_t)((buf[2] << 8) | buf[3]);
  int16_t rawAz = (int16_t)((buf[4] << 8) | buf[5]);
  // buf[6], buf[7] = TEMP (ignored)
  int16_t rawGx = (int16_t)((buf[8] << 8) | buf[9]);
  int16_t rawGy = (int16_t)((buf[10] << 8) | buf[11]);
  int16_t rawGz = (int16_t)((buf[12] << 8) | buf[13]);
  ax = (float)rawAx / 16384.0f; // g
  ay = (float)rawAy / 16384.0f;
  az = (float)rawAz / 16384.0f;
  gx = (float)rawGx / 131.0f;   // deg/s
  gy = (float)rawGy / 131.0f;
  gz = (float)rawGz / 131.0f;
  return true;
}

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println(F("Setting up Satellite Dashboard WiFi..."));

  // GPIO setup
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  digitalWrite(TRIG_PIN, LOW);

  // LDR analog input (no pinMode needed for A0)

  // I2C init
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  Wire.setClock(400000); // 400kHz fast mode
  if (!mpuInit()) {
    Serial.println(F("MPU6050 init failed"));
  } else {
    Serial.println(F("MPU6050 init OK"));
  }

  // Configure WiFi Access Point
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, netMsk);
  WiFi.softAP(ssid, password);

  Serial.print(F("WiFi AP Created: "));
  Serial.println(ssid);
  Serial.print(F("IP Address: "));
  Serial.println(WiFi.softAPIP());

  // Start DNS Server for Captive Portal
  dnsServer.start(DNS_PORT, "*", apIP);

  // Setup server routes
  server.on("/", handleRoot);
  server.on("/uptime", handleUptime);
  server.on("/distance", handleDistance);
  server.on("/imu", handleIMU);
  server.on("/light", handleLight);
  server.onNotFound(handleNotFound);

  // Start server
  server.begin();
  Serial.println(F("HTTP server started"));

  // Memory info
  Serial.print(F("Free heap: "));
  Serial.println(ESP.getFreeHeap());
  Serial.print(F("Flash chip size: "));
  Serial.println(ESP.getFlashChipSize());
}

void loop() {
  dnsServer.processNextRequest();
  server.handleClient();
}

// --------- Server Handlers ---------
void handleRoot() {
  server.send(200, F("text/html"), htmlContent);
}

void handleUptime() {
  server.send(200, F("text/plain"), String(millis() / 1000));
}

void handleDistance() {
  long distance = measureDistanceCm();
  server.send(200, F("text/plain"), String(distance));
}

void handleIMU() {
  float ax, ay, az, gx, gy, gz;
  if (mpuRead(ax, ay, az, gx, gy, gz)) {
    String json = "{\"ax\":" + String(ax, 3) + 
                  ",\"ay\":" + String(ay, 3) + 
                  ",\"az\":" + String(az, 3) + 
                  ",\"gx\":" + String(gx, 1) + 
                  ",\"gy\":" + String(gy, 1) + 
                  ",\"gz\":" + String(gz, 1) + "}";
    server.send(200, F("application/json"), json);
  } else {
    server.send(500, F("text/plain"), F("IMU read failed"));
  }
}

void handleLight() {
  int lightValue = analogRead(LDR_PIN); // Read 0-1023
  server.send(200, F("text/plain"), String(lightValue));
}

void handleNotFound() {
  server.sendHeader(F("Location"), F("http://192.168.4.1"), true);
  server.send(302, F("text/plain"), F(""));
} 
