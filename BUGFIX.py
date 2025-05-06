import tkinter as tk
from tkinter import ttk
from PIL import Image, ImageTk
import requests
from datetime import datetime

class LightControlApp(tk.Tk):
    def __init__(self):
        super().__init__()
        self.title("Light Control Application")
        self.geometry("820x650")

        # Background setup
        self.bg_image_path = "fondo.png"
        self.canvas = tk.Canvas(self, width=820, height=650)
        self.canvas.pack(fill="both", expand=True)
        self.add_background_image()

        # Main overlay frame
        self.main_frame = tk.Frame(self.canvas, bg="white")
        self.canvas.create_window((0, 0), window=self.main_frame, anchor="nw", width=820, height=650)

        self.banner_text = "Bienvenido a la aplicación de control de luces! Contacta al 2920591019"
        self.notebook = ttk.Notebook(self.main_frame)
        self.notebook.pack(expand=True, fill="both", pady=10)

        # Build pages
        self.create_main_page()
        self.create_existing_sequences_page()
        self.create_dimming_sequences_page()
        self.create_color_sequences_page()
        self.create_info_page()

    def add_background_image(self):
        try:
            img = Image.open(self.bg_image_path).resize((820, 650), Image.LANCZOS)
            self.bg_image_tk = ImageTk.PhotoImage(img)
            self.canvas.create_image(0, 0, anchor="nw", image=self.bg_image_tk)
        except Exception as e:
            print(f"Error cargando fondo: {e}")

    def send_request(self, action, value):
        ip = self.ip_address.get().strip()
        if not ip:
            print("Error: IP no configurada")
            return
        url = f"http://{ip}/?{action}={int(value)}"
        try:
            r = requests.get(url, timeout=3)
            print(f"Request {action}={value} -> {r.status_code}")
        except Exception as e:
            print(f"Error conexión: {e}")

    def add_slider(self, parent, label_text, initial, action=None, invert=False):
        ttk.Label(parent, text=label_text + ":", font=("Arial",14,"bold")).pack(pady=5)
        var = tk.IntVar(value=initial)
        lbl = ttk.Label(parent, text=f"{label_text} actual: {initial}")
        lbl.pack(pady=2)
        def on_slide(val):
            v = int(float(val))
            lbl.config(text=f"{label_text} actual: {v}")
            if action:
                send_val = 100 - v if invert else v
                self.send_request(action, send_val)
        slider = ttk.Scale(parent, from_=0, to=100, variable=var, command=on_slide)
        slider.pack(pady=5, padx=20, fill='x')
        return var, lbl

    def create_main_page(self):
        f = ttk.Frame(self.notebook)
        self.notebook.add(f, text="Principal")
        ttk.Label(f, text="Control de Luces Bar Barone", font=("Arial",18,"bold")).pack(pady=10)
        ttk.Label(f, text=f"Bahía Blanca: {datetime.now():%Y-%m-%d %H:%M:%S}", font=("Arial",12)).pack()

        row = ttk.Frame(f)
        row.pack(pady=5)
        ttk.Label(row, text="IP ESP32:", font=("Arial",14)).pack(side="left")
        self.ip_address = tk.StringVar()
        ttk.Entry(row, textvariable=self.ip_address, width=15).pack(side="left", padx=5)
        ttk.Button(row, text="Guardar IP", command=self.save_ip).pack(side="left")
        ttk.Button(row, text="Probar Conexión", command=self.test_connection).pack(side="left", padx=5)

        # Dimming General: no invert so slider high -> bright
        self.add_slider(f, "Dimming General", 80, action="dim", invert=False)
        # Velocidad: invert so slider high -> faster
        self.add_slider(f, "Intervalo (velocidad)", 15, action="interval", invert=True)
        self.add_slider(f, "Dimming Barra", 70, action="dimBarra", invert=False)

        btns = ttk.Frame(f)
        btns.pack(pady=10)
        self.btn_encender = ttk.Button(btns, text="Encender", command=lambda: self.send_request("dim", 100))
        self.btn_apagar = ttk.Button(btns, text="Apagar",  command=lambda: self.send_request("dim", 0))
        self.btn_encender.pack(side="left", padx=10)
        self.btn_apagar.pack(side="left", padx=10)
        self.enable_controls(False)

        lbl = tk.Label(f, text=self.banner_text, font=("Arial",12,"bold"), fg="blue")
        lbl.pack(pady=10)
        self.animate_banner(lbl)

    def save_ip(self):
        ip = self.ip_address.get().strip()
        parts = ip.split('.')
        if len(parts)==4 and all(p.isdigit() and 0<=int(p)<=255 for p in parts):
            print(f"IP válida guardada: {ip}")
            self.enable_controls(True)
        else:
            print("IP inválida")

    def test_connection(self):
        ip = self.ip_address.get().strip()
        if not ip:
            print("Error: IP no configurada")
            return
        url = f"http://{ip}/"
        try:
            r = requests.get(url, timeout=3)
            print(f"Test conexión {url} -> {r.status_code}")
        except Exception as e:
            print(f"Fallo test conexión: {e}")

    def create_existing_sequences_page(self):
        f = ttk.Frame(self.notebook)
        self.notebook.add(f, text="Secuencias Existentes")
        ttk.Label(f, text="Secuencias:", font=("Arial",14,"bold")).pack(pady=5)
        for i in range(1,6):
            ttk.Button(f, text=f"Secuencia {i}", command=lambda v=i: self.send_request("secuencia", v)).pack(pady=2)
        # Velocidad invertida, Intensidad invertida to match hardware
        self.add_slider(f, "Velocidad", 50, action="interval", invert=True)
        self.add_slider(f, "Intensidad", 50, action="dimValue", invert=True)
        lbl = tk.Label(f, text=self.banner_text, font=("Arial",12,"bold"), fg="blue")
        lbl.pack(pady=10)
        self.animate_banner(lbl)

    def create_dimming_sequences_page(self):
        f = ttk.Frame(self.notebook)
        self.notebook.add(f, text="Secuencias Dimming")
        for i in range(10,15):
            ttk.Button(f, text=f"Secuencia {i}", command=lambda v=i: self.send_request("secuencia", v)).pack(pady=2)
        self.add_slider(f, "Velocidad", 50, action="interval", invert=True)
        self.add_slider(f, "Intensidad", 50, action="dimValue", invert=True)
        lbl = tk.Label(f, text=self.banner_text, font=("Arial",12,"bold"), fg="blue")
        lbl.pack(pady=10)
        self.animate_banner(lbl)

    def create_color_sequences_page(self):
        f = ttk.Frame(self.notebook)
        self.notebook.add(f, text="Secuencias Colores")
        for i in range(20,25):
            ttk.Button(f, text=f"Secuencia {i}", command=lambda v=i: self.send_request("secuencia", v)).pack(pady=2)
        self.add_slider(f, "Velocidad", 50, action="interval", invert=True)
        self.add_slider(f, "Intensidad", 50, action="dimValue", invert=True)
        lbl = tk.Label(f, text=self.banner_text, font=("Arial",12,"bold"), fg="blue")
        lbl.pack(pady=10)
        self.animate_banner(lbl)

    def create_info_page(self):
        f = ttk.Frame(self.notebook)
        self.notebook.add(f, text="Info")
        txt = "Control de luces Bar Barone. Ajusta secuencias, dimming y velocidad. Para ayuda: 2920591019."
        ttk.Label(f, text=txt, wraplength=700).pack(pady=10)

    def enable_controls(self, ena):
        state = "normal" if ena else "disabled"
        for w in (self.btn_encender, self.btn_apagar):
            w.config(state=state)

    def animate_banner(self, lbl):
        def m():
            lbl.config(text=lbl.cget("text")[1:]+lbl.cget("text")[0])
            lbl.after(200,m)
        m()

if __name__ == "__main__":
    LightControlApp().mainloop()
