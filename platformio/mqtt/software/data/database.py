from pymongo import MongoClient
from pymongo.server_api import ServerApi
from dotenv import load_dotenv
import os

load_dotenv()  # Carrega as variáveis do arquivo .env

def get_database():
    try:
        MONGO_URI = os.getenv("MONGO_URI")
        if not MONGO_URI:
            raise ValueError("MONGO_URI não está definida nas variáveis de ambiente")
        client = MongoClient(MONGO_URI, server_api=ServerApi('1'))
        client.admin.command('ping')
        print("Pinged your deployment. You successfully connected to MongoDB!")
        return client["sensores_db"]  # Retorne o banco de dados desejado
    except Exception as e:
        print(e)
        return None