import hashlib
import json
import time

class Block:
    def __init__(self, index, timestamp, data, previous_hash):
        self.index = index
        self.timestamp = timestamp
        self.data = data  # Ex: {"product_id": "123ABC", "location": "Warehouse", "details": {...}}
        self.previous_hash = previous_hash
        self.hash = self.calculate_hash()

    def calculate_hash(self):
        block_string = f"{self.index}{self.timestamp}{json.dumps(self.data)}{self.previous_hash}"
        return hashlib.sha256(block_string.encode()).hexdigest()

class Blockchain:
    def __init__(self):
        self.chain = []
        self.load_chain()

    def create_genesis_block(self):
        return Block(0, time.time(), {"message": "Genesis Block"}, "0")

    def add_block(self, data):
        last_block = self.chain[-1]
        new_block = Block(len(self.chain), time.time(), data, last_block.hash)
        self.chain.append(new_block)
        self.save_chain()

    def save_chain(self):
        with open("storage.json", "w") as f:
            json.dump([block.__dict__ for block in self.chain], f, indent=2)

    def load_chain(self):
        try:
            with open("storage.json", "r") as f:
                blocks = json.load(f)
                self.chain = [
                    Block(b['index'], b['timestamp'], b['data'], b['previous_hash']) 
                    for b in blocks
                ]
        except FileNotFoundError:
            self.chain = [self.create_genesis_block()]
            self.save_chain()
