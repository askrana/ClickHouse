#pragma once

#include <ext/shared_ptr_helper.h>
#include <Formats/FormatSettings.h>
#include <Storages/IStorage.h>
#include <Storages/MergeTree/MergeTreeData.h>


namespace DB
{

class Context;

struct StoragesInfo
{
    StoragePtr storage = nullptr;
    TableStructureReadLockHolder table_lock;

    String database;
    String table;
    String engine;

    bool need_inactive_parts = false;
    MergeTreeData * data = nullptr;

    operator bool() const { return storage != nullptr; }
    MergeTreeData::DataPartsVector getParts(MergeTreeData::DataPartStateVector & state, bool has_state_column) const;
};

/** A helper class that enumerates the storages that match given query. */
class StoragesInfoStream
{
public:
    StoragesInfoStream(const SelectQueryInfo & query_info, const Context & context);
    StoragesInfo next();

private:
    String query_id;

    ColumnPtr database_column;
    ColumnPtr table_column;
    ColumnPtr active_column;

    size_t next_row;
    size_t rows;

    using StoragesMap = std::map<std::pair<String, String>, StoragePtr>;
    StoragesMap storages;
};

/** Implements system table 'parts' which allows to get information about data parts for tables of MergeTree family.
  */
class StorageSystemPartsBase : public IStorage
{
public:
    std::string getTableName() const override { return name; }

    NameAndTypePair getColumn(const String & column_name) const override;

    bool hasColumn(const String & column_name) const override;

    BlockInputStreams read(
            const Names & column_names,
            const SelectQueryInfo & query_info,
            const Context & context,
            QueryProcessingStage::Enum processed_stage,
            size_t max_block_size,
            unsigned num_streams) override;

private:
    const std::string name;

    bool hasStateColumn(const Names & column_names) const;

protected:
    const FormatSettings format_settings;

    StorageSystemPartsBase(std::string name_, NamesAndTypesList && columns_);

    virtual void processNextStorage(MutableColumns & columns, const StoragesInfo & info, bool has_state_column) = 0;
};

}
