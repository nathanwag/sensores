import serial
import datetime
import paho.mqtt.client as mqtt
from pymongo.mongo_client import MongoClient
from pymongo.server_api import ServerApi
from database import get_database

ser = serial.Serial('COM4', 115200, timeout=1)
mqtt_client = mqtt.Client(client_id="SensorGateway", callback_api_version=mqtt.CallbackAPIVersion.VERSION2)
mqtt_client.connect("localhost", 1883, 60)
db = get_database()

while True:
    try:
        if ser.in_waiting > 0:
            data = ser.readline().decode('utf-8').strip()
            temp = float(data.split(',')[0].split(':')[1])
            humi = float(data.split(',')[1].split(':')[1])
            light = float(data.split(',')[2].split(':')[1])
            atmop = float(data.split(',')[3].split(':')[1])
            turbi = float(data.split(',')[4].split(':')[1])
            mqtt_client.publish("sensors/temperature", f"{temp}", qos=0)
            mqtt_client.publish("sensors/humidity", f"{humi}", qos=0)
            mqtt_client.publish("sensors/light", f"{light}", qos=0)
            mqtt_client.publish("sensors/atmospheric", f"{atmop}", qos=0)
            mqtt_client.publish("sensors/turbinity", f"{turbi}", qos=0)
            
            dados = {
                "temperatura": temp,
                "umidade": humi,
                "luz": light,
                "pressao_atmosferica": atmop,
                "turbidez": turbi,
                "timestamp": datetime.datetime.now()
            }
            collection = db["sensores_colecao"]
            result = collection.insert_one(dados)

            print(f"Publicado: Temp={temp}, Humi={humi}, Light={light}, Atmop={atmop}, Turbi={turbi}")
    except Exception as e:
        print(f"Erro: {e}")