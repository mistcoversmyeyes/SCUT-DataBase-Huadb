//===----------------------------------------------------------------------===//
//
//                         HuaDB
//
// page_header.h
//
// Identification: src/table/page_header.h
//
//===----------------------------------------------------------------------===//

#pragma once

#include <string>

#include "common/types.h"
#include "log/log_manager.h"
#include "storage/page.h"
#include "table/record.h"

namespace huadb {

// PageHeader 占用的字节数
// page_lsn(8) + next_page(4) + page_lower(2) + page_upper(2) = 16
static constexpr db_size_t PAGE_HEADER_SIZE = sizeof(lsn_t) + sizeof(pageid_t) + sizeof(db_size_t) + sizeof(db_size_t);

class ColumnList;

/**
 * PageHeader 类是 slotted page 结构的管理者
 *
 * 页面布局（从低地址到高地址）：
 * +-----------------+  <- 0
 * | PageLSN (8B)    |  LAB 2: 用于 WAL 日志恢复
 * +-----------------+  <- 8
 * | NextPageID (4B) |  指向下一个页面，形成链表
 * +-----------------+  <- 12
 * | Lower (2B)      |  指向空闲空间的起始位置（从低地址增长）
 * +-----------------+  <- 14
 * | Upper (2B)      |  指向空闲空间的结束位置（从高地址减少）
 * +-----------------+  <- 16 (PAGE_HEADER_SIZE)
 * | Slot 0          |  槽位数组，每个槽位记录一条记录的偏移和大小
 * | Slot 1          |
 * | ...             |
 * +-----------------+  <- Lower
 * |   Free Space    |  空闲空间
 * +-----------------+  <- Upper
 * | Record N        |  记录从高地址向低地址增长
 * | ...             |
 * | Record 1        |
 * | Record 0        |
 * +-----------------+  <- DB_PAGE_SIZE
 *
 * PageHeader 作为 Page 的代理类，提供了页面操作的高层接口。
 * 所有对页面的访问都应该通过 PageHeader，而不是直接操作 Page。
 */
class PageHeader {
 public:
  explicit PageHeader(std::shared_ptr<Page> page);

  void Init();
  slotid_t InsertRecord(std::shared_ptr<Record> record, xid_t xid, cid_t cid);
  void DeleteRecord(slotid_t slot_id, xid_t xid);
  void UpdateRecordInPlace(const Record &record, slotid_t slot_id);
  std::shared_ptr<Record> GetRecord(Rid rid, const ColumnList &column_list);

  void UndoDeleteRecord(slotid_t slot_id);
  void RedoInsertRecord(slotid_t slot_id, char *raw_record, db_size_t page_offset, db_size_t record_size);

  db_size_t GetRecordCount() const;
  lsn_t GetPageLSN() const;
  pageid_t GetNextPageId() const;
  db_size_t GetLower() const;
  db_size_t GetUpper() const;
  db_size_t GetFreeSpaceSize() const;

  void SetNextPageId(pageid_t page_id);
  void SetPageLSN(lsn_t page_lsn);

  std::string ToString() const;

 private:
  char *page_data_;            // 指向页面原始数据的指针
  lsn_t *page_lsn_;            // 指向页面 LSN 的指针（LAB 2: 用于 WAL 日志）
  pageid_t *next_page_id_;     // 指向下一个页面 ID 的指针（形成页面链表）
  db_size_t *lower_;           // 指向 lower 指针（空闲空间起始位置，从低地址增长）
  db_size_t *upper_;           // 指向 upper 指针（空闲空间结束位置，从高地址减少）
  Slot *slots_;                // 指向槽位数组起始位置
  std::shared_ptr<Page> page_; // 页面对象的共享指针（保持页面生命周期）
};

}  // namespace huadb
