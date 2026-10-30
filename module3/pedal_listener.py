import socket
import time

PORT = 5005

def on_open():
    print("  -> lid opened")          # trigger music here

def on_close(duration_ms):
    print(f"  -> lid closed after {duration_ms} ms")   # e.g. map duration to a note/length

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind(("0.0.0.0", PORT))
print(f"listening on UDP {PORT}...")

opens = 0
while True:
    data, addr = sock.recvfrom(1024)
    msg = data.decode().strip()
    stamp = time.strftime("%H:%M:%S")

    if msg == "OPEN":
        opens += 1
        print(f"[{stamp}] OPEN (#{opens}) from {addr[0]}")
        on_open()
    elif msg.startswith("CLOSED"):
        ms = int(msg.split()[1])
        print(f"[{stamp}] CLOSED {ms} ms")
        on_close(ms)
    elif msg.startswith("HB"):
        print(f"[{stamp}] heartbeat, delta={msg.split()[1]}")
    else:
        print(f"[{stamp}] {msg} from {addr[0]}")