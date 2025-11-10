const $ = (s, d=document)=>d.querySelector(s);

const joinCard = $('#joinCard');
const chatCard = $('#chatCard');
const roomNameEl = $('#roomName');
const statusEl = $('#status');
const typingEl = $('#typing');
const listEl = $('#messages');

let ws = null;
let state = {
  name: localStorage.getItem('chat-name') || '',
  room: localStorage.getItem('chat-room') || 'global',
};

$('#nameInput').value = state.name || '';
$('#roomInput').value = state.room || 'global';

(function initTheme(){
  const key = 'chat-theme';
  if(localStorage.getItem(key) === 'light') document.documentElement.classList.add('light');
  $('#themeBtn').addEventListener('click', ()=>{
    document.documentElement.classList.toggle('light');
    localStorage.setItem(key, document.documentElement.classList.contains('light') ? 'light':'dark');
  });
})();

$('#clearBtn').addEventListener('click', ()=>{ listEl.innerHTML = ''; });

$('#joinBtn').addEventListener('click', ()=>{
  const name = $('#nameInput').value.trim() || 'Guest';
  const room = $('#roomInput').value.trim() || 'global';
  state.name = name; state.room = room;
  localStorage.setItem('chat-name', name);
  localStorage.setItem('chat-room', room);
  connect();
});

$('#form').addEventListener('submit', (e)=>{
  e.preventDefault();
  if(!ws || ws.readyState !== WebSocket.OPEN) return;
  const text = $('#msg').value;
  $('#msg').value = '';
  ws.send(JSON.stringify({ type:'chat', name: state.name, text }));
  addMessage({ self:true, name: state.name, text, ts: Date.now() });
});

let typingTimer = null;
$('#msg').addEventListener('input', ()=>{
  if(ws && ws.readyState === WebSocket.OPEN){
    ws.send(JSON.stringify({ type:'typing' }));
  }
});

function connect(){
  const loc = window.location;
  const proto = loc.protocol === 'https:' ? 'wss':'ws';
  const wsUrl = `${proto}://${loc.host}/chat?room=${encodeURIComponent(state.room)}&name=${encodeURIComponent(state.name)}`;
  roomNameEl.textContent = state.room;
  joinCard.classList.add('hidden');
  chatCard.classList.remove('hidden');
  statusEl.textContent = 'Đang kết nối...';

  ws = new WebSocket(wsUrl);

  ws.addEventListener('open', ()=> statusEl.textContent = 'Đã kết nối');
  ws.addEventListener('close', ()=> statusEl.textContent = 'Mất kết nối');
  ws.addEventListener('error', ()=> statusEl.textContent = 'Lỗi kết nối');

  ws.addEventListener('message', (ev)=>{
    let data;
    try { data = JSON.parse(ev.data); } catch { return; }

    if(data.type === 'welcome'){
      addSystem(`Chào mừng! Hiện có ${data.users} người trong phòng.`);
      return;
    }
    if(data.type === 'system'){
      addSystem(data.text);
      return;
    }
    if(data.type === 'typing'){
      typingEl.classList.remove('hidden');
      typingEl.textContent = `${data.name} đang nhập...`;
      clearTimeout(typingTimer);
      typingTimer = setTimeout(()=> typingEl.classList.add('hidden'), 1200);
      return;
    }
    if(data.type === 'chat'){
      addMessage(data);
      return;
    }
  });
}

function addSystem(text){
  const li = document.createElement('li');
  li.className = 'message system';
  li.textContent = text;
  listEl.appendChild(li);
  listEl.scrollTop = listEl.scrollHeight;
}

function addMessage({ name, text, ts, self=false }){
  const li = document.createElement('li');
  li.className = 'message' + (self ? ' mine' : '');

  const meta = document.createElement('div');
  meta.className = 'meta';
  const date = new Date(ts || Date.now());
  meta.textContent = `${name || 'Unknown'} • ${date.toLocaleTimeString()}`;

  const content = document.createElement('div');
  content.className = 'text';
  content.textContent = text;

  li.appendChild(meta);
  li.appendChild(content);
  listEl.appendChild(li);
  listEl.scrollTop = listEl.scrollHeight;
}

if(state.name && state.room){
  connect();
}
