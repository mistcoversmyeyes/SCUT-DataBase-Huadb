//===----------------------------------------------------------------------===//
//
//                         HuaDB
//
// table.h
//
// Identification: src/table/table.h
//
//===----------------------------------------------------------------------===//

#pragma once

#include "catalog/column_list.h"
#include "common/types.h"
#include "log/log_manager.h"
#include "storage/buffer_pool.h"
#include "table/record.h"

namespace huadb {

/**
 * Table 类表示一个基于堆文件组织的记录集合
 *
 * 表由一个页面链表组成，每个页面使用 slotted page 结构存储多条记录。
 * Table 类提供了插入、删除和更新记录的高层接口。
 *
 * 底层的页面布局由 PageHeader 类管理，PageHeader 处理槽位管理和空间分配的底层细节。
 */
class Table {
 public:
  Table(BufferPool &buffer_pool, LogManager &log_manager, oid_t oid, oid_t db_oid, ColumnList column_list,
        bool new_table, bool is_empty);

  Rid InsertRecord(std::shared_ptr<Record> record, xid_t xid, cid_t cid, bool write_log);
  void DeleteRecord(const Rid &rid, xid_t xid, bool write_log);
  Rid UpdateRecord(const Rid &rid, xid_t xid, cid_t cid, std::shared_ptr<Record> record, bool write_log);
  void UpdateRecordInPlace(const Record &record);

  pageid_t GetFirstPageId() const;
  oid_t GetTableOid() const;
  oid_t GetDbOid() const;
  const ColumnList &GetColumnList() const;

 private:
  BufferPool &buffer_pool_;    // 缓冲池引用
  LogManager &log_manager_;    // 日志管理器引用
  oid_t table_oid_;            // 表的对象标识符
  oid_t db_oid_;               // 数据库的对象标识符
  pageid_t first_page_id_;     // 第一个页面的页面号，NULL_PAGE_ID 表示表为空
  ColumnList column_list_;     // 表的 schema 信息（列定义）
};

}  // namespace huadb
