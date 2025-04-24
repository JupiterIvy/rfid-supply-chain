import json
import paho.mqtt.client as mqtt
from blockchain import Blockchain

blockchain = Blockchain()

def on_connect(client, userdata, flags, rc):
    print("MQTT conectado com código:", rc)
    client.subscribe("supplychain/rfid")

def on_message(client, userdata, msg):
    try:
        payload = json.loads(msg.payload.decode())
        print("Mensagem recebida:", payload)

        product_id = payload["product_id"]
        location = payload["location"]
        details = payload["details"]  # {"name": "...", "category": "...", "mfg_date": "...", "exp_date": "..."}

        blockchain.add_block({
            "product_id": product_id,
            "location": location,
            "details": details
        })

        print("Bloco registrado na blockchain local.")

    except Exception as e:
        print("Erro ao processar mensagem:", e)

def start_mqtt():
    client = mqtt.Client()
    client.on_connect = on_connect
    client.on_message = on_message
    client.connect("broker.hivemq.com", 1883, 60)
    client.loop_forever()
