import tkinter as tk
from tkinter import ttk
from PIL import Image, ImageTk
import requests
from datetime import datetime

class LightControlApp(tk.Tk):
    def __init__(self):
        super().__init__()
        self.title("Light Control Application")
        self.geometry("800x600")

        # Ruta de la imagen de fondo
        self.bg_image_path = "fondo.png"  # Cambia el nombre según la imagen que desees usar
        
        # Crear Canvas y configurar la imagen de fondo
        self.canvas = tk.Canvas(self, width=800, height=600)
        self.canvas.pack(fill="both", expand=True)

        # Cargar y configurar la imagen de fondo
        self.add_background_image()

        # Agregar marco principal sobre el fondo
        self.main_frame = tk.Frame(self.canvas, bg="white")
        self.canvas.create_window((0, 0), window=self.main_frame, anchor="nw", width=800, height=600)

        # Inicializar el texto del banner
        self.banner_text = "Bienvenido a la aplicación de control de luces! Contacta al 2920591019 para más información."

        # Crear las pestañas de la aplicación
        self.notebook = ttk.Notebook(self.main_frame)
        self.notebook.pack(expand=True, fill="both")

        self.create_main_page()
        self.create_existing_sequences_page()
        self.create_dimming_sequences_page()
        self.create_color_sequences_page()
        self.create_info_page()

    def add_background_image(self):
        try:
            # Cargar la imagen
            bg_image = Image.open(self.bg_image_path)
            # Redimensionar la imagen para ajustarla al tamaño de la ventana
            bg_image = bg_image.resize((800, 600), Image.LANCZOS)
            self.bg_image_tk = ImageTk.PhotoImage(bg_image)
            # Colocar la imagen en el canvas
            self.canvas.create_image(0, 0, anchor="nw", image=self.bg_image_tk)
        except Exception as e:
            print(f"Error al cargar la imagen de fondo: {e}")

    def create_main_page(self):
        main_frame = ttk.Frame(self.notebook)
        self.notebook.add(main_frame, text="Principal")

        ttk.Label(main_frame, text="Control de Luces en Bar Barone", font=("Arial", 18, "bold")).pack(pady=10)

        # Mostrar la fecha y la hora
        current_time = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        ttk.Label(main_frame, text=f"Bahía Blanca: {current_time}", font=("Arial", 12)).pack(pady=5)

        ttk.Label(main_frame, text="Ingrese la IP del ESP32:", font=("Arial", 14)).pack(pady=10)
        self.ip_address = tk.StringVar()
        ttk.Entry(main_frame, textvariable=self.ip_address).pack(pady=5)
        ttk.Button(main_frame, text="Guardar IP", command=self.save_ip).pack(pady=5)

        ttk.Label(main_frame, text="Intensidad Dimming General:", font=("Arial", 14, "bold")).pack(pady=10)
        self.dim_value = tk.IntVar(value=50)
        dimming_slider = ttk.Scale(main_frame, from_=0, to=100, variable=self.dim_value, command=self.update_dim_value)
        dimming_slider.pack(pady=5, padx=20, fill='x')
        self.dim_value_label = ttk.Label(main_frame, text=f"Valor actual: {self.dim_value.get()}")
        self.dim_value_label.pack(pady=5)

        ttk.Label(main_frame, text="Control de Dimming en Barra:", font=("Arial", 14, "bold")).pack(pady=10)
        self.dim_barra_value = tk.IntVar(value=50)
        dimming_barra_slider = ttk.Scale(main_frame, from_=0, to=100, variable=self.dim_barra_value, command=self.update_dim_barra_value)
        dimming_barra_slider.pack(pady=5, padx=20, fill='x')
        self.dim_barra_value_label = ttk.Label(main_frame, text=f"Valor de dimmer para barra: {self.dim_barra_value.get()}")
        self.dim_barra_value_label.pack(pady=5)

        ttk.Label(main_frame, text="Opciones Básicas:", font=("Arial", 14, "bold")).pack(pady=10)
        ttk.Button(main_frame, text="Encender Luces", command=lambda: self.send_request("encender", 1)).pack(pady=5)
        ttk.Button(main_frame, text="Apagar Luces", command=lambda: self.send_request("apagar", 0)).pack(pady=5)

        banner_label = tk.Label(main_frame, text=self.banner_text, font=("Arial", 12, "bold"), foreground="blue")
        banner_label.pack(pady=10)
        self.animate_banner(banner_label)

    def create_existing_sequences_page(self):
        existing_frame = ttk.Frame(self.notebook)
        self.notebook.add(existing_frame, text="Secuencias Existentes")

        ttk.Label(existing_frame, text="Selecciona una secuencia existente:", font=("Arial", 14, "bold")).pack(pady=10)

        secuencias = [
            ("Secuencia 1 ......", 1),
            ("Secuencia 2 ......", 2),
            ("Secuencia 3 ......", 3),
            ("Secuencia 4 ......", 4),
            ("Secuencia 5 ......", 5)
        ]

        for seq in secuencias:
            button = ttk.Button(existing_frame, text=seq[0], style='TButton', command=lambda s=seq[1]: self.send_request("secuencia", s))
            button.pack(pady=5)

        # Controles de velocidad e intensidad
        ttk.Label(existing_frame, text="Velocidad:", font=("Arial", 14, "bold")).pack(pady=5)
        self.speed_value = tk.IntVar(value=50)
        speed_slider = ttk.Scale(existing_frame, from_=0, to=100, variable=self.speed_value, command=self.update_speed_value)
        speed_slider.pack(pady=5, padx=20, fill='x')
        self.speed_value_label = ttk.Label(existing_frame, text=f"Velocidad actual: {self.speed_value.get()}")
        self.speed_value_label.pack(pady=5)

        ttk.Label(existing_frame, text="Intensidad:", font=("Arial", 14, "bold")).pack(pady=5)
        self.intensity_value = tk.IntVar(value=50)
        intensity_slider = ttk.Scale(existing_frame, from_=0, to=100, variable=self.intensity_value, command=self.update_intensity_value)
        intensity_slider.pack(pady=5, padx=20, fill='x')
        self.intensity_value_label = ttk.Label(existing_frame, text=f"Intensidad actual: {self.intensity_value.get()}")
        self.intensity_value_label.pack(pady=5)

        # Banner con movimiento
        banner_label = tk.Label(existing_frame, text=self.banner_text, font=("Arial", 12, "bold"), foreground="blue")
        banner_label.pack(pady=10)
        self.animate_banner(banner_label)

    def create_dimming_sequences_page(self):
        dimming_frame = ttk.Frame(self.notebook)
        self.notebook.add(dimming_frame, text="Secuencias con Dimming")

        ttk.Label(dimming_frame, text="Selecciona una secuencia con dimming:", font=("Arial", 14, "bold")).pack(pady=10)

        secuencias_dimming = [
            ("Secuencia 1 Dimming ......", 10),
            ("Secuencia 2 Dimming ......", 11),
            ("Secuencia 3 Dimming ......", 12),
            ("Secuencia 4 Dimming ......", 13),
            ("Secuencia 5 Dimming ......", 14)
        ]

        for seq in secuencias_dimming:
            button = ttk.Button(dimming_frame, text=seq[0], style='TButton', command=lambda s=seq[1]: self.send_request("secuencia", s))
            button.pack(pady=5)

        # Controles de velocidad e intensidad
        ttk.Label(dimming_frame, text="Velocidad:", font=("Arial", 14, "bold")).pack(pady=5)
        self.speed_value = tk.IntVar(value=50)
        speed_slider = ttk.Scale(dimming_frame, from_=0, to=100, variable=self.speed_value, command=self.update_speed_value)
        speed_slider.pack(pady=5, padx=20, fill='x')
        self.speed_value_label = ttk.Label(dimming_frame, text=f"Velocidad actual: {self.speed_value.get()}")
        self.speed_value_label.pack(pady=5)

        ttk.Label(dimming_frame, text="Intensidad:", font=("Arial", 14, "bold")).pack(pady=5)
        self.intensity_value = tk.IntVar(value=50)
        intensity_slider = ttk.Scale(dimming_frame, from_=0, to=100, variable=self.intensity_value, command=self.update_intensity_value)
        intensity_slider.pack(pady=5, padx=20, fill='x')
        self.intensity_value_label = ttk.Label(dimming_frame, text=f"Intensidad actual: {self.intensity_value.get()}")
        self.intensity_value_label.pack(pady=5)

        # Banner con movimiento
        banner_label = tk.Label(dimming_frame, text=self.banner_text, font=("Arial", 12, "bold"), foreground="blue")
        banner_label.pack(pady=10)
        self.animate_banner(banner_label)

    def create_color_sequences_page(self):
        color_frame = ttk.Frame(self.notebook)
        self.notebook.add(color_frame, text="Secuencias con Colores")

        ttk.Label(color_frame, text="Selecciona una secuencia con colores:", font=("Arial", 14, "bold")).pack(pady=10)

        secuencias_colores = [
            ("Secuencia 1 Colores ......", 20),
            ("Secuencia 2 Colores ......", 21),
            ("Secuencia 3 Colores ......", 22),
            ("Secuencia 4 Colores ......", 23),
            ("Secuencia 5 Colores ......", 24)
        ]

        for seq in secuencias_colores:
            button = ttk.Button(color_frame, text=seq[0], style='TButton', command=lambda s=seq[1]: self.send_request("secuencia", s))
            button.pack(pady=5)

        # Controles de velocidad e intensidad
        ttk.Label(color_frame, text="Velocidad:", font=("Arial", 14, "bold")).pack(pady=5)
        self.speed_value = tk.IntVar(value=50)
        speed_slider = ttk.Scale(color_frame, from_=0, to=100, variable=self.speed_value, command=self.update_speed_value)
        speed_slider.pack(pady=5, padx=20, fill='x')
        self.speed_value_label = ttk.Label(color_frame, text=f"Velocidad actual: {self.speed_value.get()}")
        self.speed_value_label.pack(pady=5)

        ttk.Label(color_frame, text="Intensidad:", font=("Arial", 14, "bold")).pack(pady=5)
        self.intensity_value = tk.IntVar(value=50)
        intensity_slider = ttk.Scale(color_frame, from_=0, to=100, variable=self.intensity_value, command=self.update_intensity_value)
        intensity_slider.pack(pady=5, padx=20, fill='x')
        self.intensity_value_label = ttk.Label(color_frame, text=f"Intensidad actual: {self.intensity_value.get()}")
        self.intensity_value_label.pack(pady=5)

        # Banner con movimiento
        banner_label = tk.Label(color_frame, text=self.banner_text, font=("Arial", 12, "bold"), foreground="blue")
        banner_label.pack(pady=10)
        self.animate_banner(banner_label)

    def create_info_page(self):
        info_frame = ttk.Frame(self.notebook)
        self.notebook.add(info_frame, text="Info")

        ttk.Label(info_frame, text="Información sobre el control de luces:", font=("Arial", 14, "bold")).pack(pady=10)

        info_text = (
            "Esta aplicación permite el control de las luces del Bar Barone. "
            "Puedes seleccionar secuencias de iluminación existentes, ajustar la intensidad "
            "del dimming, y controlar las luces de manera individual o en barra. "
            "Para más información, contacta al 2920591019."
        )

        ttk.Label(info_frame, text=info_text, wraplength=750).pack(pady=10)

        # Banner con movimiento
        banner_label = tk.Label(info_frame, text=self.banner_text, font=("Arial", 12, "bold"), foreground="blue")
        banner_label.pack(pady=10)
        self.animate_banner(banner_label)

    def update_dim_value(self, event):
        self.dim_value_label.config(text=f"Valor actual: {self.dim_value.get()}")

    def update_dim_barra_value(self, event):
        self.dim_barra_value_label.config(text=f"Valor de dimmer para barra: {self.dim_barra_value.get()}")

    def update_speed_value(self, event):
        self.speed_value_label.config(text=f"Velocidad actual: {self.speed_value.get()}")

    def update_intensity_value(self, event):
        self.intensity_value_label.config(text=f"Intensidad actual: {self.intensity_value.get()}")

    def send_request(self, action, value):
        try:
            ip = self.ip_address.get()
            url = f"http://{ip}/{action}/{value}"
            response = requests.get(url)
            if response.status_code == 200:
                print(f"Acción '{action}' enviada exitosamente con valor {value}.")
            else:
                print(f"Error al enviar la acción '{action}' con valor {value}. Código de estado: {response.status_code}")
        except Exception as e:
            print(f"Error al enviar la solicitud: {e}")

    def save_ip(self):
        ip = self.ip_address.get()
        print(f"IP guardada: {ip}")

    def animate_banner(self, label):
        def marquee():
            text = label.cget("text")
            text = text[1:] + text[0]
            label.config(text=text)
            label.after(200, marquee)

        marquee()

if __name__ == "__main__":
    app = LightControlApp()
    app.mainloop()
