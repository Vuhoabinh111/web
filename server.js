import express from 'express';
import { WebSocketServer } from 'ws';
import http from 'http';
import url from 'url';

const app = express();
const server = http.createServer(app);

// Serve static client files
app.use(express.static(new URL('../client', import.meta.url).pathname));

// Health check
app.get('/health', (req, res) => res.json({ ok: true }));

// Create WebSocket server
const wss = new WebSocketServer({ noServer: true });

// Track clients by room
const rooms = new Map(); // roomName -> Set of ws

function getRoom(room) {
  if (!rooms.has(room)) rooms.set(room, new Set());
  return rooms.get(room);
}

function broadcast(room, data, exceptWs = null) {
  const set = rooms.get(room);
  if (!set) return;
  const msg = JSON.stringify(data);
  for (const ws of set) {
    if (ws !== exceptWs && ws.readyState === ws.OPEN) {
      ws.send(msg);
    }
  }
}

// Handle protocol upgrade requests and route to WS server if path is /chat
server.on('upgrade', (request, socket, head) => {
  const { pathname, query } = url.parse(request.url, true);
  if (pathname === '/chat') {
    wss.handleUpgrade(request, socket, head, (ws) => {
      wss.emit('connection', ws, request, query);
    });
  } else {
    socket.destroy();
  }
});

wss.on('connection', (ws, request, query) => {
  const room = (query.room || 'global').toString();
  const name = (query.name || 'Guest').toString().slice(0, 30);

  const roomSet = getRoom(room);
  roomSet.add(ws);

  // Notify others
  broadcast(room, { type: 'system', text: `${name} đã tham gia phòng`, ts: Date.now() });

  // Send welcome + room population
  ws.send(JSON.stringify({ type: 'welcome', room, users: roomSet.size, ts: Date.now() }));

  ws.on('message', (raw) => {
    try {
      const data = JSON.parse(raw.toString());
      if (data.type === 'chat' && typeof data.text === 'string') {
        const trimmed = data.text.trim();
        if (trimmed.length > 0 && trimmed.length <= 2000) {
          broadcast(room, {
            type: 'chat',
            name: data.name?.toString().slice(0, 30) || name,
            text: trimmed,
            ts: Date.now()
          });
        }
      } else if (data.type === 'typing') {
        broadcast(room, { type: 'typing', name, ts: Date.now() }, ws);
      }
    } catch (e) {
      // ignore malformed
    }
  });

  ws.on('close', () => {
    const set = rooms.get(room);
    if (set) {
      set.delete(ws);
      if (set.size === 0) rooms.delete(room);
      broadcast(room, { type: 'system', text: `${name} đã rời phòng`, ts: Date.now() });
    }
  });
});

const PORT = process.env.PORT || 3000;
server.listen(PORT, () => {
  console.log('Chat server running on http://localhost:' + PORT);
});
