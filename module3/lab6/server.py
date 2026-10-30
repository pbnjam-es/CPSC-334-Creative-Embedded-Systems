import socket
import tkinter as tk
import random
HOST = "0.0.0.0"
PORT = 5000

root = tk.Tk()
root.geometry("400x300")
    
def handle_client():
    value = random.randint(0, 4096)
    value /= 16
    rgb = (value, value, value)
    hex_color = f"#{int(rgb[0]):02x}{int(rgb[1]):02x}{int(rgb[2]):02x}"
    root.configure(bg=hex_color)
    root.after(50, handle_client)
    print(hex_color)

handle_client()
root.mainloop()

#     with conn:
#         conn.settimeout(5)
#         buffer = ""
#         try:
#             while True:
#                 chunk = conn.recv(1024)
#                 if not chunk:
#                     break
#                 buffer += chunk;
#             print(buffer)
#         except socket.timeout:
#             print("Timed out")

# with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as server:
#         server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
#         server.bind((HOST, PORT))
#         server.listen(5)
#         print(f"Listening on port {PORT}")
#         
#         try:
#             while True:
#                 conn, addr = server.accept();
#                 handle_client(conn, addr)
#         except:
#             pass
