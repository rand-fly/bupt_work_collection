#pragma once
#include <memory>
#include <string>

enum class UserType {
    base = 0,
    challenger = 1,
    author = 2
};

class User;
using UserPtr = std::shared_ptr<User>;

class User {
  public:
    User(const std::string &name, const std::string &password, int level = 1)
        : m_name(name), m_password(password), m_level(level) {}

    std::string getName() const { return m_name; }
    std::string getPassword() const { return m_password; }
    int getLevel() const { return m_level; }
    virtual UserType getType() const { return UserType::base; }

    virtual std::string serialize() const;

    static UserPtr deserialize(const std::string &str);

  protected:
    std::string m_name, m_password;
    int m_level;
};

class Challenger : public User {
  public:
    Challenger(const std::string &name, const std::string &password, int level = 1, int exp = 0, int levelPassed = 0)
        : User(name, password, level), m_exp(exp), m_levelPassed(levelPassed) {}

    virtual UserType getType() const override { return UserType::challenger; }
    std::string getInfo() const;
    virtual std::string serialize() const override;

    int getExp() const { return m_exp; }

    int getExpForNextLevel() const;

    void addExp(int exp);

    int getLevelPassed() const { return m_levelPassed; }

    void passLevel();

  private:
    int m_exp;
    int m_levelPassed;
};

class Author : public User {
  public:
    Author(const std::string &name, const std::string &password, int level = 1, int problemsMade = 0)
        : User(name, password, level), m_madeNum(problemsMade) {}

    virtual UserType getType() const override { return UserType::author; }
    std::string getInfo() const;
    virtual std::string serialize() const override;

    int getMadeNum() const { return m_madeNum; }

    int getMadeNumForNextLevel() const;

    void addProblem();

  private:
    int m_madeNum;
};