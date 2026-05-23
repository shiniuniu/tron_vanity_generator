#pragma once

#include <cstdint>
#include <cstring>
#include <string>
#include <vector>
#include <array>
#include <memory>

namespace tron {

// TRON address constants
constexpr size_t PRIVATE_KEY_SIZE = 32;
constexpr size_t PUBLIC_KEY_SIZE = 65;  // Uncompressed
constexpr size_t PUBLIC_KEY_COMPRESSED_SIZE = 33;
constexpr size_t ADDRESS_SIZE = 21;  // 20 bytes + 1 version byte
constexpr size_t ADDRESS_CHECKSUM_SIZE = 25;
constexpr size_t ADDRESS_STRING_SIZE = 34;  // Base58 encoded

// Cryptographic hash sizes
constexpr size_t SHA256_SIZE = 32;
constexpr size_t RIPEMD160_SIZE = 20;
constexpr size_t KECCAK256_SIZE = 32;

// TRON network parameters
constexpr uint8_t TRON_ADDRESS_VERSION = 0x41;  // Mainnet version byte
constexpr uint8_t TRON_ADDRESS_VERSION_TESTNET = 0x05;

// Type definitions
using PrivateKey = std::array<uint8_t, PRIVATE_KEY_SIZE>;
using PublicKey = std::array<uint8_t, PUBLIC_KEY_SIZE>;
using PublicKeyCompressed = std::array<uint8_t, PUBLIC_KEY_COMPRESSED_SIZE>;
using Address = std::array<uint8_t, ADDRESS_SIZE>;
using Hash256 = std::array<uint8_t, SHA256_SIZE>;
using Hash160 = std::array<uint8_t, RIPEMD160_SIZE>;
using AddressString = std::array<char, ADDRESS_STRING_SIZE>;

/**
 * @struct KeyPair
 * @brief Represents a complete TRON keypair with derived address
 * Cache-aligned for optimal CPU performance
 */
struct alignas(64) KeyPair {
    PrivateKey private_key;
    PublicKeyCompressed public_key;
    Address address;
    AddressString address_str;
    uint64_t timestamp;
    
    KeyPair() : timestamp(0) {
        std::memset(private_key.data(), 0, PRIVATE_KEY_SIZE);
        std::memset(public_key.data(), 0, PUBLIC_KEY_COMPRESSED_SIZE);
        std::memset(address.data(), 0, ADDRESS_SIZE);
        std::memset(address_str.data(), 0, ADDRESS_STRING_SIZE);
    }
};

/**
 * @class AddressGenerator
 * @brief High-performance TRON address generation with cryptographic operations
 * 
 * Features:
 * - Batch key generation
 * - CPU multithreading support
 * - GPU acceleration via CUDA
 * - Lock-free operations where possible
 * - Memory pool integration
 * - Cache-friendly data structures
 */
class AddressGenerator {
public:
    AddressGenerator();
    ~AddressGenerator();

    /**
     * Generate a single keypair
     * @return Newly generated KeyPair
     */
    KeyPair generate_keypair();

    /**
     * Generate multiple keypairs (batch operation)
     * @param count Number of keypairs to generate
     * @return Vector of KeyPairs
     */
    std::vector<KeyPair> generate_batch(size_t count);

    /**
     * Generate keypair from specific private key
     * @param private_key The private key bytes
     * @return Derived KeyPair
     */
    KeyPair keypair_from_private_key(const PrivateKey& private_key);

    /**
     * Securely wipe private key from memory
     * @param key Private key to wipe
     */
    static void secure_wipe_key(PrivateKey& key);

    /**
     * Get address generation rate (addresses/second)
     * @return Current generation rate
     */
    double get_generation_rate() const;

    /**
     * Reset performance counters
     */
    void reset_stats();

private:
    // secp256k1 operations
    PublicKeyCompressed derive_public_key(const PrivateKey& private_key);
    Address derive_address(const PublicKeyCompressed& public_key);
    std::string address_to_base58(const Address& address);

    // Hash operations
    Hash256 sha256(const uint8_t* data, size_t len);
    Hash160 ripemd160(const uint8_t* data, size_t len);
    Hash256 keccak256(const uint8_t* data, size_t len);

    // Statistics
    mutable uint64_t total_generated = 0;
    mutable uint64_t start_time = 0;
};

}  // namespace tron
