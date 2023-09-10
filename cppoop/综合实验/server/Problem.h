#pragma once
#include <string>

class Problem {
  public:
    Problem(const std::string &word) : m_word(word) {}
    std::string word() const { return m_word; }
    int length() const { return (int)m_word.length(); }
    bool empty() const { return m_word.empty(); }
    std::string serialize() const { return m_word; }
    static Problem deserialize(const std::string &str) { return Problem(str); }

  private:
    std::string m_word;
};