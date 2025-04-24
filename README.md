# Supply Chain Tracker com RFID, MQTT e Blockchain Local

Este projeto é um DApp (aplicativo descentralizado) de rastreamento de produtos, mais especificamente frutas como o Tucumã, na cadeia de suprimentos utilizando:

- Leitor **RFID** com **ESP32**
- Comunicação via **MQTT** (broker HiveMQ)
- Backend em **Python** com uma **blockchain local**
- Frontend com **Streamlit** para visualização em tempo real

## Funcionalidades

- Registro de produtos na blockchain local
- Leitura de etiquetas RFID reais via ESP32
- Envio automático de leituras para o backend via MQTT
- Atualização automática do histórico de localização dos produtos
- Interface visual com Streamlit exibindo:
  - Detalhes do produto
  - Última localização
  - Histórico completo como cards interativos

---

## Estrutura do Projeto

```bash
supplychain-project/
├── backend/
│   ├── blockchain.py            # Implementação da blockchain local
│   ├── mqtt_listener.py         # Subscreve ao MQTT e registra os dados
│   └── contract_utils.py        # Utilitários para manipular produtos
├── esp32/
│   └── rfid_mqtt.ino            # Código da ESP32 com leitura RFID e MQTT
├── frontend/
│   └── app.py                   # Interface em Streamlit
├── storage.json                 # Blockchain salva em arquivo local
├── read-rfid.ino
├── README.md
```

# Equipe

<table>
<tr>
    <td align="center"><a href="https://github.com/giseledesa"><img src="https://media.licdn.com/dms/image/v2/C4D03AQH4CX61Pe9A_g/profile-displayphoto-shrink_400_400/profile-displayphoto-shrink_400_400/0/1605043825215?e=1750896000&v=beta&t=mmxSgmnxYO421bOqxsIxI-3WhEaEmr4oxpqDf2kO0eI" width="100px;" alt=""/><br /><sub><b>Gisele de Sá</b></sub></a><br /></td> 
    <td align="center"><a href="https://github.com/JupiterIvy"><img src="https://avatars.githubusercontent.com/u/65917017?v=4" width="100px;" alt=""/><br /><sub><b>Evelyn Bessa</b></sub></a><br /></td>
    <td align="center"><a href="https://github.com/belavalcacer"><img src="https://avatars.githubusercontent.com/u/120135756?v=4" width="100px;" alt=""/><br /><sub><b>Sandra Valcacer</b></sub></a><br /></td>
</table>
