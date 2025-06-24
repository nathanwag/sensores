import serial
import matplotlib.pyplot as plt
import matplotlib.animation as animation
from datetime import datetime

# Configuração da porta serial (substitua 'COM4' pela porta correta)
try:
    ser = serial.Serial('COM4', baudrate=115200, timeout=1)
except serial.SerialException as e:
    print(f"Erro ao abrir a porta serial: {e}")
    exit(1)

# Listas para armazenar dados para o gráfico
latitudes = []
longitudes = []
times = []

# Configuração do gráfico
fig, ax = plt.subplots()
line, = ax.plot([], [], 'b-', label='Trajetória')  # Gráfico de linha
plt.xlabel('Longitude')
plt.ylabel('Latitude')
plt.title('Trajetória do GPS em Tempo Real')
plt.grid(True)
plt.legend()

# Definir limites iniciais (ajustáveis dinamicamente)
ax.set_xlim(-180, 180)
ax.set_ylim(-90, 90)

def update_plot(frame):
    """Função chamada para atualizar o gráfico"""
    try:
        while True:
            line = ser.readline().decode('utf-8', errors='ignore').strip()
            if line.startswith("=== Dados do GPS ==="):
                # Lê um bloco de dados até o marcador final
                data = {}
                while True:
                    line = ser.readline().decode('utf-8', errors='ignore').strip()
                    if line == "====================":
                        break
                    if line:
                        key_value = line.split(": ", 1)
                        if len(key_value) == 2:
                            key, value = key_value
                            data[key] = value
                
                # Exibir os dados no console
                print("-" * 40)
                print(f"Data: {data.get('Data', 'N/A')}")
                print(f"Hora: {data.get('Hora', 'N/A')}")
                print(f"Latitude: {data.get('Latitude', 'N/A')}")
                print(f"Longitude: {data.get('Longitude', 'N/A')}")
                print(f"Altitude: {data.get('Altitude', 'N/A')}")
                print(f"HDOP: {data.get('HDOP', 'N/A')}")
                print(f"Velocidade: {data.get('Velocidade', 'N/A')}")
                print("-" * 40)
                
                # Extrair e validar dados
                try:
                    lat = float(data.get('Latitude', 'N/A'))
                    lon = float(data.get('Longitude', 'N/A'))
                    if lat != 0 and lon != 0:  # Ignora coordenadas inválidas
                        latitudes.append(lat)
                        longitudes.append(lon)
                        
                        # Parse da data e hora do GPS
                        date_str = data.get('Data', 'N/A')
                        time_str = data.get('Hora', 'N/A')
                        if date_str != 'N/A' and time_str != 'N/A':
                            date_parts = date_str.split('-')
                            time_parts = time_str.split(':')
                            if len(date_parts) == 3 and len(time_parts) == 3:
                                YY, DD, MM = map(int, date_parts)
                                HH, MI, SS = map(int, time_parts)
                                full_year = 2000 + YY  # Assume anos entre 2000 e 2099
                                try:
                                    gps_time = datetime(full_year, MM, DD, HH, MI, SS)
                                    times.append(gps_time)
                                except ValueError:
                                    print("Data ou hora inválida, pulando timestamp...")
                        
                        # Atualizar o gráfico
                        line.set_data(longitudes, latitudes)
                        
                        # Ajustar os limites do gráfico dinamicamente
                        if latitudes and longitudes:
                            ax.set_xlim(min(longitudes) - 0.001, max(longitudes) + 0.001)
                            ax.set_ylim(min(latitudes) - 0.001, max(latitudes) + 0.001)
                        
                        # Redesenhar o gráfico
                        fig.canvas.draw()
                        fig.canvas.flush_events()
                except ValueError:
                    print("Erro: Coordenadas inválidas, pulando...")
                break  # Sai do loop após processar um bloco completo
    except serial.SerialException as e:
        print(f"Erro de comunicação serial: {e}")
        return

# Configurar a animação para atualizar o gráfico
ani = animation.FuncAnimation(fig, update_plot, interval=5000)  # Intervalo de 5 segundos

print("Lendo dados do GPS e plotando... Pressione Ctrl+C para parar.")

try:
    plt.show()  # Exibir o gráfico
except KeyboardInterrupt:
    print("Parado.")
finally:
    ser.close()