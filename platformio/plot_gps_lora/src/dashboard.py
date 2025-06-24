import serial
import matplotlib.pyplot as plt
import matplotlib.animation as animation
from datetime import datetime

# Configuração da porta serial
try:
    ser = serial.Serial('COM3', baudrate=115200, timeout=1)
except serial.SerialException as e:
    print(f"Erro ao abrir a porta serial: {e}")
    exit(1)

# Listas para armazenar os dados
latitudes = []
longitudes = []
times = []

# Configuração do gráfico
fig, ax = plt.subplots()
line, = ax.plot([], [], 'b-', label='Trajetória')
plt.xlabel('Longitude')
plt.ylabel('Latitude')
plt.title('Trajetória do GPS em Tempo Real')
plt.grid(True)
plt.legend()
ax.set_xlim(-180, 180)  # Limites iniciais amplos
ax.set_ylim(-90, 90)

def update_plot(frame):
    try:
        line_data = ser.readline().decode('utf-8', errors='ignore').strip()
        if line_data:
            print(f"Received: {line_data}")  # Debug
            try:
                # Divide a linha em latitude e longitude
                lat_str, lon_str = line_data.split(',')
                lat = float(lat_str.strip())
                lon = float(lon_str.strip())
                
                # Verifica se as coordenadas são válidas
                if lat != 0 and lon != 0:
                    latitudes.append(lat)
                    longitudes.append(lon)
                    times.append(datetime.now())
                    
                    # Atualiza os limites do gráfico dinamicamente
                    if latitudes and longitudes:
                        ax.set_xlim(min(longitudes) - 0.001, max(longitudes) + 0.001)
                        ax.set_ylim(min(latitudes) - 0.001, max(latitudes) + 0.001)
                        line.set_data(longitudes, latitudes)
                else:
                    print("Coordenadas zero detectadas, pulando...")
            except ValueError as ve:
                print(f"Erro ao converter coordenadas: {ve}, pulando...")
            except Exception as e:
                print(f"Erro ao processar linha: {e}, pulando...")
    except serial.SerialException as e:
        print(f"Erro de comunicação serial: {e}")
    return line,

# Configuração da animação
ani = animation.FuncAnimation(fig, update_plot, interval=2000, blit=True)
print("Lendo dados do GPS e plotando... Pressione Ctrl+C para parar.")

try:
    plt.show()
except KeyboardInterrupt:
    print("Parado pelo usuário.")
finally:
    ser.close()
    print("Porta serial fechada.")