#pragma once

#include <string>
#include <vector>
#include <array>
#include <memory>
#include <regex>
#include "address_generator.h"

namespace tron {

/**
 * @enum MatchType
 * @brief Types of vanity matching rules
 */
enum class MatchType {
    PREFIX,      // Match at start of address
    SUFFIX,      // Match at end of address
    CONTAINS,    // Match anywhere in address
};

/**
 * @struct VanityRule
 * @brief Single vanity matching rule
 */
struct VanityRule {
    MatchType type;
    std::string pattern;
    bool case_sensitive;
    bool found;
    
    VanityRule(MatchType t, const std::string& p, bool cs = true)
        : type(t), pattern(p), case_sensitive(cs), found(false) {}
};

/**
 * @class VanityMatcher
 * @brief High-performance vanity address pattern matching
 * 
 * Features:
 * - Multiple simultaneous rules
 * - Case-sensitive/insensitive matching
 * - SIMD optimized string comparison
 * - Lock-free rule checking
 * - Pattern caching
 */
class VanityMatcher {
public:
    VanityMatcher();
    ~VanityMatcher();

    /**
     * Add a vanity matching rule
     * @param rule The VanityRule to add
     */
    void add_rule(const VanityRule& rule);

    /**
     * Add multiple rules at once
     * @param rules Vector of VanityRules
     */
    void add_rules(const std::vector<VanityRule>& rules);

    /**
     * Clear all rules
     */
    void clear_rules();

    /**
     * Check if an address matches any rule
     * @param address The TRON address string to check
     * @return Matched rule index, -1 if no match
     */
    int check_address(const std::string& address);

    /**
     * Check if address matches specific rule
     * @param address The TRON address string
     * @param rule_index Index of rule to check
     * @return True if matches
     */
    bool check_rule(const std::string& address, size_t rule_index);

    /**
     * Get matched rule
     * @return The matching VanityRule or nullptr
     */
    const VanityRule* get_matched_rule() const;

    /**
     * Get total number of rules
     * @return Rule count
     */
    size_t rule_count() const;

    /**
     * Get rule by index
     * @param index Rule index
     * @return Pointer to VanityRule or nullptr
     */
    const VanityRule* get_rule(size_t index) const;

    /**
     * Get statistics: how many addresses checked
     * @return Total checked count
     */
    uint64_t get_checked_count() const;

    /**
     * Reset statistics
     */
    void reset_stats();

private:
    // SIMD-optimized string operations
    bool simd_equals(const char* s1, const char* s2, size_t len);
    bool simd_case_insensitive_equals(const char* s1, const char* s2, size_t len);
    
    // Pattern matching with various optimization techniques
    bool match_prefix(const std::string& address, const VanityRule& rule);
    bool match_suffix(const std::string& address, const VanityRule& rule);
    bool match_contains(const std::string& address, const VanityRule& rule);

    std::vector<VanityRule> rules;
    int last_matched_index = -1;
    mutable uint64_t checked_count = 0;
};

}  // namespace tron
