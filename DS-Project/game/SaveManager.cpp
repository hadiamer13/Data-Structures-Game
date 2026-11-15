#include "SaveManager.h"

#include <fstream>
#include <sstream>

#include "Json.h"
#include "../model/Serializer.h"
#include "../model/Validator.h"

namespace
{
    const char* kindName(int kind)
    {
        if (kind == TreeKind::Binary) return "binary";
        if (kind == TreeKind::BST) return "bst";
        if (kind == TreeKind::AVL) return "avl";
        if (kind == TreeKind::MaxHeap) return "maxheap";
        return nullptr;
    }

    bool kindFromName(const std::string& name, int& kind)
    {
        if (name == "binary") kind = TreeKind::Binary;
        else if (name == "bst") kind = TreeKind::BST;
        else if (name == "avl") kind = TreeKind::AVL;
        else if (name == "maxheap") kind = TreeKind::MaxHeap;
        else return false;
        return true;
    }

    bool integerField(const JsonValue* object, const char* key, int& value)
    {
        const JsonValue* field = object == nullptr ? nullptr : object->get(key);
        return field != nullptr && field->asInteger(value);
    }

    bool stringField(const JsonValue* object, const char* key, std::string& value)
    {
        const JsonValue* field = object == nullptr ? nullptr : object->get(key);
        return field != nullptr && field->asString(value);
    }

    void writeIntArray(std::ostringstream& output, const IntArray& values)
    {
        output << '[';
        for (int i = 0; i < values.size(); i++)
        {
            if (i > 0) output << ',';
            output << values[i];
        }
        output << ']';
    }

    bool validateEndlessTree(const SaveData& data, std::string* error)
    {
        if (data.endlessTreeKind == TreeKind::MaxHeap)
        {
            if (!data.endlessTree.empty())
            {
                if (error != nullptr) *error = "Heap saves must use the heap array instead of a tree snapshot";
                return false;
            }
            MaxHeap heap;
            if (!heap.loadState(data.endlessHeap, data.activeSize))
            {
                if (error != nullptr) *error = "Saved heap property, size, or sorted region is invalid";
                return false;
            }
            return true;
        }
        if (data.endlessHeap.size() != 0 || data.activeSize != 0)
        {
            if (error != nullptr) *error = "Tree saves must not contain heap state";
            return false;
        }
        if (data.endlessTreeKind == TreeKind::Binary)
        {
            BinaryTree tree;
            std::string detail;
            if (!Serializer::deserializeBinaryTree(data.endlessTree, tree) || !Validator::validateBinaryTree(tree, &detail))
            {
                if (error != nullptr) *error = detail.empty() ? "Saved binary tree is invalid" : detail;
                return false;
            }
            return true;
        }
        if (data.endlessTreeKind == TreeKind::BST)
        {
            BST tree;
            std::string detail;
            if (!Serializer::deserializeBST(data.endlessTree, tree) || !Validator::validateBST(tree, &detail))
            {
                if (error != nullptr) *error = detail.empty() ? "Saved BST is invalid" : detail;
                return false;
            }
            return true;
        }
        if (data.endlessTreeKind == TreeKind::AVL)
        {
            AVLTree tree;
            std::string detail;
            if (!Serializer::deserializeAVL(data.endlessTree, tree) || !Validator::validateAVL(tree, &detail))
            {
                if (error != nullptr) *error = detail.empty() ? "Saved AVL tree is invalid" : detail;
                return false;
            }
            return true;
        }
        if (error != nullptr) *error = "Saved endless tree kind is unknown";
        return false;
    }
}

SaveData::SaveData() : endlessSeed(0), endlessTreeKind(TreeKind::AVL),
                       endlessTree("# "), activeSize(0), bestScore(0)
{
    for (int i = 0; i < 20; i++) stars.push(0);
}

bool SaveManager::validate(const SaveData& data, std::string* error)
{
    if (error != nullptr) error->clear();
    if (data.stars.size() != 20)
    {
        if (error != nullptr) *error = "Progress must contain exactly 20 star entries";
        return false;
    }
    for (int i = 0; i < data.stars.size(); i++)
    {
        if (data.stars[i] < 0 || data.stars[i] > 3)
        {
            if (error != nullptr) *error = "Star values must be between 0 and 3";
            return false;
        }
    }
    if (data.endlessSeed < 0 || data.bestScore < 0)
    {
        if (error != nullptr) *error = "Endless seed and score must be non-negative";
        return false;
    }
    return validateEndlessTree(data, error);
}

bool SaveManager::save(const std::string& path, const SaveData& data, std::string* error)
{
    if (error != nullptr) error->clear();
    std::string validationError;
    if (!validate(data, &validationError))
    {
        if (error != nullptr) *error = validationError;
        return false;
    }
    const char* kind = kindName(data.endlessTreeKind);
    std::ostringstream output;
    output << "{\n  \"version\": 1,\n  \"stars\": ";
    writeIntArray(output, data.stars);
    output << ",\n  \"endless\": {\n    \"seed\": " << data.endlessSeed
           << ",\n    \"kind\": " << JsonWriter::quote(kind)
           << ",\n    \"tree\": " << JsonWriter::quote(data.endlessTreeKind == TreeKind::MaxHeap ? "" : data.endlessTree)
           << ",\n    \"heap\": ";
    writeIntArray(output, data.endlessHeap);
    output << ",\n    \"activeSize\": " << data.activeSize
           << ",\n    \"bestScore\": " << data.bestScore << "\n  }\n}\n";
    std::ofstream file(path.c_str(), std::ios::binary | std::ios::trunc);
    if (!file)
    {
        if (error != nullptr) *error = std::string("Unable to open save file for writing: ") + path;
        return false;
    }
    file << output.str();
    if (!file)
    {
        if (error != nullptr) *error = std::string("Failed while writing save file: ") + path;
        return false;
    }
    return true;
}

bool SaveManager::load(const std::string& path, SaveData& out, std::string* error)
{
    if (error != nullptr) error->clear();
    std::ifstream file(path.c_str(), std::ios::binary);
    if (!file)
    {
        if (error != nullptr) *error = std::string("Unable to open save file: ") + path;
        return false;
    }
    std::ostringstream contents;
    contents << file.rdbuf();
    JsonDocument document;
    JsonError parseError;
    if (!JsonParser::parse(contents.str(), document, &parseError))
    {
        if (error != nullptr)
        {
            std::ostringstream message;
            message << parseError.message << " at character " << parseError.position;
            *error = message.str();
        }
        return false;
    }
    const JsonValue* root = document.root();
    const JsonValue* stars = root == nullptr ? nullptr : root->get("stars");
    const JsonValue* endless = root == nullptr ? nullptr : root->get("endless");
    int version = 0;
    SaveData loaded;
    if (!integerField(root, "version", version) || version != 1 || stars == nullptr ||
        stars->type() != JsonType::Array || stars->size() != 20)
    {
        if (error != nullptr) *error = "Save version or campaign star array is invalid";
        return false;
    }
    loaded.stars.clear();
    for (int i = 0; i < stars->size(); i++)
    {
        int value = 0;
        if (!stars->at(i)->asInteger(value))
        {
            if (error != nullptr) *error = "Campaign stars must be integers";
            return false;
        }
        loaded.stars.push(value);
    }
    std::string kind;
    if (!integerField(endless, "seed", loaded.endlessSeed) ||
        !stringField(endless, "kind", kind) || !kindFromName(kind, loaded.endlessTreeKind) ||
        !stringField(endless, "tree", loaded.endlessTree) ||
        !integerField(endless, "activeSize", loaded.activeSize) ||
        !integerField(endless, "bestScore", loaded.bestScore))
    {
        if (error != nullptr) *error = "Endless save fields are missing or invalid";
        return false;
    }
    const JsonValue* heap = endless->get("heap");
    if (heap == nullptr || heap->type() != JsonType::Array)
    {
        if (error != nullptr) *error = "Endless heap must be an array";
        return false;
    }
    loaded.endlessHeap.clear();
    for (int i = 0; i < heap->size(); i++)
    {
        int value = 0;
        if (!heap->at(i)->asInteger(value))
        {
            if (error != nullptr) *error = "Endless heap entries must be integers";
            return false;
        }
        loaded.endlessHeap.push(value);
    }
    if (!validate(loaded, error)) return false;
    out.stars = loaded.stars;
    out.endlessSeed = loaded.endlessSeed;
    out.endlessTreeKind = loaded.endlessTreeKind;
    out.endlessTree = loaded.endlessTree;
    out.endlessHeap = loaded.endlessHeap;
    out.activeSize = loaded.activeSize;
    out.bestScore = loaded.bestScore;
    return true;
}
