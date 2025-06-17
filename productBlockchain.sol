// SPDX-License-Identifier: MIT
pragma solidity ^0.8.7;

contract ProductBlockchain {
    struct Block {
        uint index;
        uint timestamp;
        string productId;
        string location;
        string details;
    }

    Block[] public blockchain;

    constructor() {
        blockchain.push(Block(0, block.timestamp, "GENESIS", "Origin", "Genesis Block"));
    }

    function addBlock(string memory productId, string memory location, string memory details) public {
        uint index = blockchain.length;
        blockchain.push(Block(index, block.timestamp, productId, location, details));
    }

    function getBlock(uint i) public view returns (uint, uint, string memory, string memory, string memory) {
        Block memory b = blockchain[i];
        return (b.index, b.timestamp, b.productId, b.location, b.details);
    }

    function getChainLength() public view returns (uint) {
        return blockchain.length;
    }
}
