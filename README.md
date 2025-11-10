# Simple Chat App (WebSocket)

Ứng dụng chat thời gian thực, hỗ trợ phòng (room).

## Chạy trên máy
1. Cài Node.js (>= 18).
2. Mở terminal trong thư mục `chat-app`:
   ```bash
   npm install
   npm start
   ```
3. Vào http://localhost:3000, nhập nickname + room, rồi chat (mở 2 tab).

## Ghi chú bảo mật
- Demo, **không mã hoá end‑to‑end**.
- Nếu public, hãy bổ sung: giới hạn tốc độ, đăng nhập, lưu DB, HTTPS.
