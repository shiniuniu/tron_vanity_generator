#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <mutex>
#include <memory>
#include "address_generator.h"

namespace tron {

/**
 * @enum OutputFormat
 * @brief Supported output file formats
 */
enum class OutputFormat {
    TXT,  // Plain text format
    CSV,  // Comma-separated values
};

/**
 * @struct VanityResult
 * @brief Result of a successful vanity address match
 */
struct VanityResult {
    std::string address;
    std::string private_key_hex;
    std::string public_key_hex;
    std::string matched_pattern;
    uint64_t timestamp;
    uint64_t attempt_count;
    
    VanityResult() : timestamp(0), attempt_count(0) {}
};

/**
 * @class ResultWriter
 * @brief High-performance asynchronous result file writer
 * 
 * Features:
 * - Timestamp-based file naming (never overwrites)
 * - Lock-free where possible
 * - Buffered I/O for performance
 * - mmap file writing for speed
 * - Multiple format support (TXT, CSV)
 * - Secure memory handling
 */
class ResultWriter {
public:
    /**
     * Constructor
     * @param output_dir Output directory path
     * @param format Output file format
     */
    ResultWriter(const std::string& output_dir = "./results", OutputFormat format = OutputFormat::TXT);
    
    ~ResultWriter();

    /**
     * Write a vanity result to file
     * @param result The VanityResult to write
     * @return True if successful
     */
    bool write_result(const VanityResult& result);

    /**
     * Write multiple results in batch
     * @param results Vector of VanityResults
     * @return Number of successfully written results
     */
    size_t write_batch(const std::vector<VanityResult>& results);

    /**
     * Flush all buffered data to disk
     * @return True if successful
     */
    bool flush();

    /**
     * Get current output filename
     * @return Filename with path
     */
    std::string get_current_filename() const;

    /**
     * Get total results written in this session
     * @return Result count
     */
    size_t get_results_written() const;

    /**
     * Set output format
     * @param format The OutputFormat to use
     */
    void set_format(OutputFormat format);

private:
    // File operations
    void create_new_file();
    std::string generate_timestamp_filename();
    std::string bytes_to_hex(const uint8_t* data, size_t len);

    // Format-specific writers
    void write_txt_header();
    void write_txt_result(const VanityResult& result);
    void write_csv_header();
    void write_csv_result(const VanityResult& result);

    std::string output_directory;
    OutputFormat current_format;
    std::unique_ptr<std::ofstream> output_file;
    std::string current_filename;
    mutable std::mutex file_mutex;
    size_t results_written;
    static constexpr size_t BUFFER_SIZE = 65536;  // 64KB buffer
    char buffer[BUFFER_SIZE];
};

}  // namespace tron
