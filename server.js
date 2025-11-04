const express = require('express');
const { WebSocketServer } = require('ws');
const http = require('http');
const path = require('path');

const app = express();
const server = http.createServer(app);
const wss = new WebSocketServer({ server });



//npx localtunnel --port 3000



app.set('view engine', 'ejs');
app.set('views', path.join(__dirname, 'views'));
app.use(express.static('public'));

let latestData = {}; 

app.get('/', (req, res) => {
  res.render('dashboard', { data: latestData });
});

wss.on('connection', (ws) => {
  console.log('Client connected via WebSocket');

  ws.on('message', (message) => {
    try {
      const data = JSON.parse(message.toString());
      latestData = data;
      console.log('Received from ESP32:', data);

      wss.clients.forEach((client) => {
        if (client.readyState === ws.OPEN) {
          client.send(JSON.stringify(data));
        }
      });
    } catch (e) {
      console.error('Invalid message', e);
    }
  });

  ws.on('close', () => console.log('Client disconnected'));
});

const PORT = 3000;
server.listen(PORT, () => console.log(`Server running on http://localhost:${PORT}`));
