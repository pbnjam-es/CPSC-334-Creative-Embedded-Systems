import socket, time
from pythonosc.udp_client import SimpleUDPClient

PORT = 5005
osc = SimpleUDPClient("127.0.0.1", 57120)
# osc = SimpleUDPClient("10.67.67.61", 57120)

# ---- calibration ----
COVERED  = [1500, 1800, 1500]   # light below this (lid open) = sensor covered by trash
MIN_THUD = 0                   # below this = lid opened with no trash

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind(("0.0.0.0", PORT))
print(f"listening on UDP {PORT}...")

last_fill = 0.0
while True:
    data, addr = sock.recvfrom(1024)
    p = data.decode().strip().split()
    stamp = time.strftime("%H:%M:%S")

    if p[0] == "DEPOSIT":
        ms, thud = int(p[1]), int(p[2])
        depth, push, rise = float(p[3]), float(p[4]), int(p[5])
        light = [int(x) for x in p[6:9]]
        fill = sum(l < c for l, c in zip(light, COVERED)) / 3

        print(f"[{stamp}] DEPOSIT open={ms}ms thud={thud} depth={depth:.2f} "
              f"push={push:.2f} rise={rise}ms light={light} fill={fill:.2f}")

        if last_fill > 0 and fill == 0:
            print("  -> HARVEST")
            osc.send_message("/compost/harvest", 0)
        if thud >= MIN_THUD:
            osc.send_message("/compost/deposit", [ms, thud, depth, push, rise, fill])
        else:
            print("  (no thud: opened without trash)")
        osc.send_message("/compost/fill", fill)
        last_fill = fill

    elif p[0] == "HB":
        print(f"[{stamp}] HB raw={p[1]} depth={p[2]} push={p[3]} thud={p[4]} light={p[5:8]}")
    else:
        print(f"[{stamp}] {' '.join(p)}")