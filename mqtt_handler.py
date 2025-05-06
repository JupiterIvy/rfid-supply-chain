import json
import os
import time
from dotenv import load_dotenv
import traceback
from web3 import Web3
import paho.mqtt.client as mqtt

load_dotenv()

GANACHE_URL = os.getenv("GANACHE_URL")
PRIVATE_KEY = os.getenv("PRIVATE_KEY")
CONTRACT_ADDRESS = os.getenv("CONTRACT_ADDRESS")
with open("ProductBlockchain_abi.json", "r") as f:
    CONTRACT_ABI = json.load(f)

w3 = Web3(Web3.HTTPProvider(GANACHE_URL))
account = w3.eth.account.from_key(PRIVATE_KEY)
contract = w3.eth.contract(address=CONTRACT_ADDRESS, abi=CONTRACT_ABI)

def on_connect(client, userdata, flags, rc):
    print("MQTT conectado:", rc)
    client.subscribe("supplychain/rfid")

def on_message(client, userdata, msg):
    if msg.retain:
        print("Ignorado retido:", msg.payload.decode())
        return

    try:
        payload = json.loads(msg.payload.decode())
        print("Recebido:", payload)

        product_id = payload["product_id"]
        location = payload["location"]
        details = json.dumps(payload["details"])

        tx = contract.functions.addBlock(product_id, location, details).build_transaction({
            'from': account.address,
            'nonce': w3.eth.get_transaction_count(account.address),
            'gas': 200000,
            'gasPrice': w3.to_wei('1', 'gwei')
        })

        signed_tx = w3.eth.account.sign_transaction(tx, private_key=PRIVATE_KEY)
        tx_hash = w3.eth.send_raw_transaction(signed_tx.raw_transaction)
        print("TX enviado:", tx_hash.hex())

    except Exception as e:
        print("Erro:", e)

def start_mqtt():
    client = mqtt.Client()
    client.on_connect = on_connect
    client.on_message = on_message
    client.connect("broker.hivemq.com", 1883, 60)
    client.loop_forever()

if __name__ == "__main__":
    print("Iniciando MQTT handler...")
    start_mqtt()