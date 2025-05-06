import streamlit as st
import datetime
import json
import os
from dotenv import load_dotenv
from web3 import Web3

load_dotenv(dotenv_path=".env")

GANACHE_URL = os.getenv("GANACHE_URL")
CONTRACT_ADDRESS = os.getenv("CONTRACT_ADDRESS")
print(CONTRACT_ADDRESS)
with open("ProductBlockchain_abi.json", "r") as f:
    CONTRACT_ABI = json.load(f)

w3 = Web3(Web3.HTTPProvider(GANACHE_URL))
contract = w3.eth.contract(address=CONTRACT_ADDRESS, abi=CONTRACT_ABI)

st.set_page_config(page_title="Supply Chain Tracker", layout="wide")
st.title("Tucuma Fruit Supply Chain DApp")

length = contract.functions.getChainLength().call()

if length <= 1:
    st.warning("Nenhum produto registrado ainda. Aguardando leitura do RFID...")
    st.stop()

product_map = {}

for i in range(1, length):  # skip genesis
    _, ts, pid, location, details_str = contract.functions.getBlock(i).call()
    details = json.loads(details_str)
    product_map[pid] = {"location": location, "timestamp": ts, "details": details}

cols = st.columns(len(product_map))
selected_id = None

for idx, (pid, data) in enumerate(product_map.items()):
    with cols[idx]:
        st.markdown("------")
        if st.button(f"{data['details']['name']}", key=pid):
            selected_id = pid
        st.write(f"**Local:** {data['location']}")
        st.write(f"**ID:** `{pid}`")
        st.markdown("------")

if selected_id:
    st.markdown(f"## Histórico do Produto `{selected_id}`")
    for i in range(1, length):
        _, ts, pid, location, details_str = contract.functions.getBlock(i).call()
        if pid != selected_id:
            continue
        details = json.loads(details_str)
        dt = datetime.datetime.fromtimestamp(ts).strftime("%Y-%m-%d %H:%M:%S")
        st.markdown(f"### 📌 Local: **{location}** — ⏱️ `{dt}`")
        st.json({
            "product_id": pid,
            "location": location,
            "details": details
        })

length = contract.functions.getChainLength().call()
print(f"Number of blocks: {length}")

for i in range(length):
    block_data = contract.functions.getBlock(i).call()
    print(block_data)
