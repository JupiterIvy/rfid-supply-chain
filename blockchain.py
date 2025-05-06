import hashlib
import json
import time
import os

class Block:
    def __init__(self, index, timestamp, data, previous_hash):
        self.index = index
        self.timestamp = timestamp
        self.data = data  # Ex: {"product_id": "...", "location": "...", "details": {...}}
        self.previous_hash = previous_hash
        self.hash = self.calculate_hash()

    def calculate_hash(self):
        block_string = f"{self.index}{self.timestamp}{json.dumps(self.data, sort_keys=True)}{self.previous_hash}"
        return hashlib.sha256(block_string.encode()).hexdigest()

    def to_dict(self):
        return {
            "index": self.index,
            "timestamp": self.timestamp,
            "data": self.data,
            "previous_hash": self.previous_hash,
            "hash": self.hash
        }

class Blockchain:
    def __init__(self, storage_file="storage.json"):
        self.chain = []
        self.storage_file = storage_file
        self.load_chain()

    def create_genesis_block(self):
        print("Genesis block created.")
        return Block(0, time.time(), {"message": "Genesis Block"}, "0")

    def add_block(self, data):
        if not self.chain:
            # Garante a criação do bloco gênesis se a cadeia estiver vazia
            genesis_block = self.create_genesis_block()
            self.chain.append(genesis_block)

        last_block = self.chain[-1]
        new_block = Block(len(self.chain), time.time(), data, last_block.hash)

        # Evita duplicação: hash ou dados iguais ao último
        if new_block.hash == last_block.hash:
            print("Duplicate block. Ignoring.")
            return

        self.chain.append(new_block)
        self.save_chain()
        print("New block added.")

    def save_chain(self):
        with open(self.storage_file, "w") as f:
            json.dump([block.to_dict() for block in self.chain], f, indent=2)

    def load_chain(self):
        if not os.path.exists(self.storage_file):
            print("Storage file not found. Creating new blockchain with genesis block.")
            self.chain = [self.create_genesis_block()]
            self.save_chain()
            return

        try:
            with open(self.storage_file, "r") as f:
                blocks = json.load(f)
                if not blocks:
                    raise ValueError("Empty blockchain file.")

                self.chain = [
                    Block(b['index'], b['timestamp'], b['data'], b['previous_hash'])
                    for b in blocks
                ]
        except (json.JSONDecodeError, ValueError):
            print("Corrupted or empty blockchain file. Resetting.")
            self.chain = [self.create_genesis_block()]
            self.save_chain()
