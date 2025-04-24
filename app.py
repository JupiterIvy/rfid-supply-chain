import streamlit as st
import json
import datetime

st.set_page_config(page_title="Supply Chain Tracker", layout="wide")

# Carrega os dados da blockchain local
def load_chain():
    try:
        with open("storage.json", "r") as f:
            return json.load(f)
    except FileNotFoundError:
        return []

chain = load_chain()

# Evita falhas se o storage estiver vazio ou só com bloco gênesis
if len(chain) <= 1:
    st.title("Fruit Supply Chain DApp")
    st.warning("Nenhum produto registrado ainda. Aguardando leitura do RFID...")
    st.stop()

# Indexa os produtos com seu último checkpoint
product_map = {}
for block in chain[1:]:  # ignora bloco gênesis
    product_id = block["data"]["product_id"]
    product_map[product_id] = block["data"]

st.title("📦 Supply Chain DApp (Blockchain Local)")
st.markdown("### 🧾 Produtos Registrados")

cols = st.columns(len(product_map))
selected_id = None

# Cards horizontais
for idx, (product_id, data) in enumerate(product_map.items()):
    with cols[idx]:
        st.markdown("------")
        if st.button(f"🔍 {data['details']['name']}", key=product_id):
            selected_id = product_id
        st.write(f"**Local:** {data['location']}")
        st.write(f"**ID:** `{product_id}`")
        st.markdown("------")

# Exibe histórico ao clicar
if selected_id:
    st.markdown(f"## Histórico do Produto `{selected_id}`")
    history = [
        block for block in chain 
        if block["data"].get("product_id") == selected_id
    ]

    for block in history:
        dt = datetime.datetime.fromtimestamp(block["timestamp"]).strftime("%Y-%m-%d %H:%M:%S")
        st.markdown(f"### 📌 Local: **{block['data']['location']}** — ⏱️ `{dt}`")
        st.json(block)
