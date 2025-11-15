//===----------------------------------------------------------------------===//
//
//                         HuaDB
//
// table.cpp
//
// Identification: src/table/table.cpp
//
//===----------------------------------------------------------------------===//

#include "table/table.h"

#include "common/constants.h"
#include "common/types.h"
#include "database/connection.h"
#include "storage/buffer_pool.h"
#include "table/page_header.h"

namespace huadb {

Table::Table(BufferPool &buffer_pool, LogManager &log_manager, oid_t table_oid, oid_t db_oid, ColumnList column_list,
             bool new_table, bool is_empty)
    : buffer_pool_(buffer_pool),
      log_manager_(log_manager),
      table_oid_(table_oid),
      db_oid_(db_oid),
      column_list_(std::move(column_list)) {
  // 新表或空表的 first_page_id_ 设为 NULL_PAGE_ID，否则设为 0
  if (new_table || is_empty) {
    first_page_id_ = NULL_PAGE_ID;
  } else {
    first_page_id_ = 0;
  }
}

/**
 * 向表中插入一条记录
 *
 * 实现逻辑：
 * 1. 检查记录大小是否超过最大限制
 * 2. 遍历页面链表，查找有足够空间的页面
 * 3. 如果所有页面都满了，创建新页面并链接到链表末尾
 * 4. 使用 PageHeader 在页面中插入记录
 * 5. 如果 write_log 为 true，写入 InsertLog 和 NewPageLog（LAB 2）
 *
 * @param record 要插入的记录
 * @param xid 事务 ID（LAB 3）
 * @param cid 命令 ID（LAB 3）
 * @param write_log 是否写日志。系统表不写日志，用户表写日志（LAB 2）
 * @return 插入记录的 RID
 *
 * LAB 1 实现要点：
 * - 使用 buffer_pool_.GetPage() 获取页面
 * - 使用 PageHeader 类作为代理操作页面内容
 * - Page 只是对 char* 和 dirty_bits 的数据抽象，不要直接操作 Page
 * - 创建新页面时需要：
 *   1. 设置前一个页面的 next_page_id
 *   2. 调用 PageHeader::Init() 初始化新页面
 *   3. 更新 first_page_id_（如果是第一个页面）
 *
 * LAB 2 实现要点：
 * - 插入记录时写 InsertLog
 * - 创建新页面时写 NewPageLog
 * - 设置页面的 page_lsn
 */
Rid Table::InsertRecord(std::shared_ptr<Record> record, xid_t xid, cid_t cid, bool write_log) {
  // 检查记录大小
  if (record->GetSize() > MAX_RECORD_SIZE) {
    throw DbException("Record size too large: " + std::to_string(record->GetSize()));
  }

  // LAB 2: 当 write_log 参数为 true 时开启写日志功能
  // 在插入记录时增加写 InsertLog 过程
  // 在创建新的页面时增加写 NewPageLog 过程
  // 设置页面的 page lsn
  // LAB 2 BEGIN



  // LAB 1: 实现插入逻辑
  // 1. 遍历 PageHeader 中的 SlotsArray，判断页面是否有足够的空间插入记录
  // 2. 如果没有则通过 buffer_pool_ 创建新页面
  // 3. 如果 first_page_id_ 为 NULL_PAGE_ID，说明表还没有页面，需要创建新页面
  // 4. 创建新页面时需设置前一个页面的 next_page_id，并将新页面初始化
  // 5. 找到空间足够的页面后，通过 PageHeader 插入记录
  // 6. 返回插入记录的 rid
  // LAB 1 BEGIN(Done)
  
  Rid ret = Rid();

  // 如果 table 为空，那么调用 BufferPool::NewPage() 从缓冲区中创建一个页面
  if (first_page_id_ == NULL_PAGE_ID){
    first_page_id_ = 0;
    auto first_page = buffer_pool_.NewPage(db_oid_, table_oid_,  first_page_id_);
    auto page_header = PageHeader(first_page);
    
    auto slot_id = page_header.InsertRecord(record, xid, cid);
    ret = {first_page_id_,slot_id};
  }
  
  // table 不为空，遍历 page 链表直到结束，找到一个空位插入
  pageid_t cur_page_id = first_page_id_;
  auto cur_page_header = PageHeader(buffer_pool_.GetPage(db_oid_, table_oid_, cur_page_id));
  // 对链表中只有一个页的情况做特殊判断
  if (cur_page_header.GetFreeSpaceSize() > record->GetSize()){
    auto slot_id = cur_page_header.InsertRecord(record, xid, cid);
    ret = {cur_page_id, slot_id};
    return ret;
  }
  while (cur_page_header.GetNextPageId() != NULL_PAGE_ID) {
    if (cur_page_header.GetFreeSpaceSize() > record->GetSize()){
      auto slot_id = cur_page_header.InsertRecord(record, xid, cid);
      ret = {cur_page_id, slot_id};
      return ret;
    }

    cur_page_id = cur_page_header.GetNextPageId();
    cur_page_header = PageHeader(buffer_pool_.GetPage(db_oid_, table_oid_, cur_page_id));
  }
  
  // 现有的页面没有空位了，新建一个页面然后将记录插入到新建的页面中
  pageid_t new_page_id = cur_page_id + 1;
  auto new_page = buffer_pool_.NewPage(db_oid_, table_oid_, new_page_id);
  auto new_page_header = PageHeader(new_page);
  slotid_t slot_id = new_page_header.InsertRecord(record, xid, cid);
  ret = {new_page_id, slot_id};
  
  return ret;
}

/**
 * 删除一条记录
 *
 * LAB 1 实现要点：
 * - 根据 rid 获取对应的页面
 * - 使用 PageHeader 标记记录为已删除
 *
 * LAB 2 实现要点：
 * - 增加写 DeleteLog 过程
 * - 设置页面的 page_lsn
 *
 * LAB 3 实现要点：
 * - 通过 xid 标记删除（MVCC）
 */
void Table::DeleteRecord(const Rid &rid, xid_t xid, bool write_log) {
  // LAB 2: 增加写 DeleteLog 过程
  // 设置页面的 page lsn
  // LAB 2 BEGIN

  // LAB 1: 使用 PageHeader 操作页面
  // LAB 1 BEGIN
}

/**
 * 更新一条记录（通过删除后插入实现）
 *
 * 这是一个简单的实现，先删除旧记录，再插入新记录。
 * 注意：这会导致 RID 发生变化。
 */
Rid Table::UpdateRecord(const Rid &rid, xid_t xid, cid_t cid, std::shared_ptr<Record> record, bool write_log) {
  DeleteRecord(rid, xid, write_log);
  return InsertRecord(record, xid, cid, write_log);
}

/**
 * 系统表的原地更新（不改变 RID）
 *
 * 仅用于系统表维护，不涉及日志和事务。
 * 直接覆盖原记录的内容，不改变记录位置。
 */
void Table::UpdateRecordInPlace(const Record &record) {
  auto rid = record.GetRid();
  auto page_header = std::make_unique<PageHeader>(buffer_pool_.GetPage(db_oid_, table_oid_, rid.page_id_));
  page_header->UpdateRecordInPlace(record, rid.slot_id_);
}

pageid_t Table::GetFirstPageId() const { return first_page_id_; }

oid_t Table::GetTableOid() const { return table_oid_; }

oid_t Table::GetDbOid() const { return db_oid_; }

const ColumnList &Table::GetColumnList() const { return column_list_; }

}  // namespace huadb
