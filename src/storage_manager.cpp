#include "pch.h"
#include "storage_manager.h"

// FileStorage implementation

FileStorage::FileStorage(const std::string& filename) : storageFile(filename) {
    LoadFromFile();
}

void FileStorage::LoadFromFile() const {
    cache.clear();
    std::ifstream file(storageFile);
    if (!file) return;

    std::string key, value;
    while (file >> key >> value) {
        cache[key] = value;
    }
}

void FileStorage::SaveToFile() const {
    std::ofstream file(storageFile, std::ios::trunc);
    if (!file) return;

    for (const auto& [key, value] : cache) {
        file << key << " " << value << "\n";
    }
}

bool FileStorage::Save(const std::string& key, const std::string& value) {
    cache[key] = value;
    SaveToFile();
    return true;
}

std::optional<std::string> FileStorage::Load(const std::string& key) const {
    auto it = cache.find(key);
    return (it != cache.end()) ? std::optional(it->second) : std::nullopt;
}

bool FileStorage::Remove(const std::string& key) {
    if (cache.erase(key)) {
        SaveToFile();
        return true;
    }
    return false;
}

void FileStorage::Clear() {
    cache.clear();
    SaveToFile();
}

// MemoryStorage implementation

bool MemoryStorage::Save(const std::string& key, const std::string& value) {
    storage[key] = value;
    return true;
}

std::optional<std::string> MemoryStorage::Load(const std::string& key) const {
    auto it = storage.find(key);
    return (it != storage.end()) ? std::optional(it->second) : std::nullopt;
}

bool MemoryStorage::Remove(const std::string& key) {
    return storage.erase(key) > 0;
}

void MemoryStorage::Clear() {
    storage.clear();
}