#pragma once
#include <optional>
#include <unordered_map>
#include <fstream>
#include <filesystem>

#ifndef ISTORAGE_H
#define ISTORAGE_H

class IStorage {
public:
    virtual ~IStorage() = default;

    // Сохранить значение по ключу
    virtual bool Save(const std::string& key, const std::string& value) = 0;

    // Получить значение по ключу
    virtual std::optional<std::string> Load(const std::string& key) const = 0;

    // Удалить значение по ключу
    virtual bool Remove(const std::string& key) = 0;

    // Очистить все значения
    virtual void Clear() = 0;
};

class FileStorage : public IStorage {
public:
    explicit FileStorage(const std::string& filename);

    bool Save(const std::string& key, const std::string& value) override;
    std::optional<std::string> Load(const std::string& key) const override;
    bool Remove(const std::string& key) override;
    void Clear() override;

private:
    std::string storageFile;
    mutable std::unordered_map<std::string, std::string> cache;

    void LoadFromFile() const;
    void SaveToFile() const;
};


class MemoryStorage : public IStorage {
public:
    bool Save(const std::string& key, const std::string& value) override;
    std::optional<std::string> Load(const std::string& key) const override;
    bool Remove(const std::string& key) override;
    void Clear() override;

private:
    std::unordered_map<std::string, std::string> storage;
};

#endif // ISTORAGE_H

