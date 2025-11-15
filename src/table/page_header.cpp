//===----------------------------------------------------------------------===//
//
//                         HuaDB
//
// page_header.cpp
//
// Identification: src/table/page_header.cpp
//
//===----------------------------------------------------------------------===//

#include "table/page_header.h"

#include <sstream>
#include "common/types.h"
#include "storage/page.h"
#include "table/record.h"
#include "table/table_page.h"

namespace huadb {

/**
 * 构造 PageHeader 对象，将页面数据映射到内存指针
 *
 * 通过 reinterpret_cast 将页面原始数据按照 slotted page 布局解析为各个字段的指针。
 * 这样可以直接通过指针操作页面内容，而无需手动计算偏移。
 *
 * @param page 页面对象的共享指针
 */
PageHeader::PageHeader(std::shared_ptr<Page> page) : page_(page) {
  page_data_ = page->GetData();
  db_size_t offset = 0;
  // 按照页面布局依次设置各字段指针
  page_lsn_ = reinterpret_cast<lsn_t *>(page_data_);
  offset += sizeof(lsn_t);
  next_page_id_ = reinterpret_cast<pageid_t *>(page_data_ + offset);
  offset += sizeof(pageid_t);
  lower_ = reinterpret_cast<db_size_t *>(page_data_ + offset);
  offset += sizeof(db_size_t);
  upper_ = reinterpret_cast<db_size_t *>(page_data_ + offset);
  offset += sizeof(db_size_t);
  assert(offset == PAGE_HEADER_SIZE);
  slots_ = reinterpret_cast<Slot *>(page_data_ + PAGE_HEADER_SIZE);
}

/**
 * 初始化页面
 *
 * 将页面设置为初始状态：
 * - page_lsn = 0
 * - next_page_id = NULL_PAGE_ID（表示这是最后一个页面）
 * - lower = PAGE_HEADER_SIZE（槽位数组为空）
 * - upper = DB_PAGE_SIZE（没有记录）
 * - 标记页面为脏页
 */
void PageHeader::Init() {
  *page_lsn_ = 0;
  *next_page_id_ = NULL_PAGE_ID;
  *lower_ = PAGE_HEADER_SIZE;
  *upper_ = DB_PAGE_SIZE;
  page_->SetDirty();
}

/**
 * 在页面中插入一条记录
 *
 * LAB 1 实现要点：
 * 1. 从高地址分配记录空间（upper 向下移动）
 * 2. 从低地址分配槽位空间（lower 向上移动）
 * 3. 设置 slots 数组（记录偏移和大小）
 * 4. 将 record 序列化到页面数据
 * 5. 标记页面为脏页
 * 6. 返回插入的 slot id
 *
 * LAB 3 实现要点：
 * - 在记录头添加事务信息（xid 和 cid）
 *
 * @param record 要插入的记录
 * @param xid 事务 ID（LAB 3）
 * @param cid 命令 ID（LAB 3）
 * @return 插入的槽号
 */
slotid_t PageHeader::InsertRecord(std::shared_ptr<Record> record, xid_t xid, cid_t cid) {
  // LAB 3: 在记录头添加事务信息（xid 和 cid）
  // LAB 3 BEGIN

  // LAB 1: 实现插入逻辑
  // 维护 lower 和 upper 指针
  // 设置 slots 数组
  // 将 record 写入 page data
  // 将 page 标记为 dirty
  // 返回插入的 slot id
  // LAB 1 BEGIN
  
  // 获取新 slot 的指针
  db_size_t new_slot_offset = *lower_;
  Slot* new_slot = reinterpret_cast<Slot*>(page_data_ + new_slot_offset);
  *lower_ += sizeof(Slot);

  db_size_t new_record_offset = *upper_ - record->GetSize();
  Record* new_record = reinterpret_cast<Record*>(page_data_ + new_record_offset);
  *upper_ -= record->GetSize();

  *new_slot = {new_record_offset, record->GetSize()};
  *new_record = Record(*record);

  page_->SetDirty();

  return 0;
}

/**
 * 删除一条记录
 *
 * LAB 1 实现要点：
 * - 将 slot_id 对应的 record 标记为删除
 * - 可使用 Record::DeserializeHeaderFrom 函数读取记录头
 * - 将页面标记为脏页
 *
 * LAB 3 实现要点：
 * - 通过 xid 标记删除（MVCC），而不是直接删除
 *
 * @param slot_id 要删除的记录的槽号
 * @param xid 事务 ID（LAB 3）
 */
void PageHeader::DeleteRecord(slotid_t slot_id, xid_t xid) {
  // LAB 3: 更改实验 1 的实现，改为通过 xid 标记删除
  // LAB 3 BEGIN

  // LAB 1: 将 slot_id 对应的 record 标记为删除
  // 可使用 Record::DeserializeHeaderFrom 函数读取记录头
  // 将 page 标记为 dirty
  // LAB 1 BEGIN
}

/**
 * 系统表的原地更新（不改变记录位置）
 *
 * 仅用于系统表维护，直接覆盖原记录内容。
 * 不涉及日志和事务。
 *
 * @param record 新的记录内容
 * @param slot_id 要更新的记录的槽号
 */
void PageHeader::UpdateRecordInPlace(const Record &record, slotid_t slot_id) {
  record.SerializeTo(page_data_ + slots_[slot_id].offset_);
  page_->SetDirty();
}

/**
 * 获取一条记录
 *
 * LAB 1 实现要点：
 * 1. 根据 slot_id 从 slots 数组获取记录的偏移和大小
 * 2. 从页面数据中反序列化记录
 * 3. 新建 Record 对象并设置 rid
 * 4. 返回记录的共享指针
 *
 * @param rid 记录标识符（包含页面号和槽号）
 * @param column_list 表的 schema 信息（用于反序列化）
 * @return 记录对象的共享指针
 */
std::shared_ptr<Record> PageHeader::GetRecord(Rid rid, const ColumnList &column_list) {
  // LAB 1: 根据 slot_id 获取 record
  // 新建 record 并设置 rid
  // LAB 1 BEGIN
  return nullptr;
}

/**
 * LAB 2: 回滚删除操作（undo）
 *
 * 清除记录的删除标记，恢复记录的可见性。
 * 用于 WAL 日志恢复过程中的 undo 操作。
 *
 * LAB 3: 修改 undo delete 的逻辑，使用 MVCC 机制
 *
 * @param slot_id 要恢复的记录的槽号
 */
void PageHeader::UndoDeleteRecord(slotid_t slot_id) {
  // LAB 3: 修改 undo delete 的逻辑
  // LAB 3 BEGIN

  // LAB 2: 清除记录的删除标记
  // 将页面设为 dirty
  // LAB 2 BEGIN
}

/**
 * LAB 2: 重做插入操作（redo）
 *
 * 将记录写入页面指定位置，用于 WAL 日志恢复过程中的 redo 操作。
 * 需要维护页面的 lower、upper 指针和 slots 数组。
 *
 * @param slot_id 要插入的槽号
 * @param raw_record 原始记录数据
 * @param page_offset 记录在页面中的偏移
 * @param record_size 记录大小
 */
void PageHeader::RedoInsertRecord(slotid_t slot_id, char *raw_record, db_size_t page_offset, db_size_t record_size) {
  // LAB 2: 将 raw_record 写入 page data
  // 注意维护 lower 和 upper 指针，以及 slots 数组
  // 将页面设为 dirty
  // LAB 2 BEGIN
}

/**
 * 获取页面中的记录数目
 * @return 记录数目（通过 lower 指针计算槽位数量）
 */
db_size_t PageHeader::GetRecordCount() const { return (*lower_ - PAGE_HEADER_SIZE) / sizeof(Slot); }

/**
 * LAB 2: 获取页面的 LSN（Log Sequence Number）
 * @return 页面的 LSN
 */
lsn_t PageHeader::GetPageLSN() const { return *page_lsn_; }

/**
 * 获取下一个页面的页面号
 * @return 下一个页面的页面号（NULL_PAGE_ID 表示这是最后一个页面）
 */
pageid_t PageHeader::GetNextPageId() const { return *next_page_id_; }

/**
 * 获取页面 lower 指针
 * @return lower 指针的值（空闲空间起始位置）
 */
db_size_t PageHeader::GetLower() const { return *lower_; }

/**
 * 获取页面 upper 指针
 * @return upper 指针的值（空闲空间结束位置）
 */
db_size_t PageHeader::GetUpper() const { return *upper_; }

/**
 * 计算页面剩余空间大小
 *
 * 空闲空间 = upper - lower - sizeof(Slot)
 * 需要减去一个 Slot 的大小，因为插入新记录时需要额外的槽位。
 *
 * @return 可用于插入记录的空间大小（字节）
 */
db_size_t PageHeader::GetFreeSpaceSize() const {
  if (*upper_ < *lower_ + sizeof(Slot)) {
    return 0;
  } else {
    return *upper_ - *lower_ - sizeof(Slot);
  }
}

/**
 * 设置下一个页面的页面号
 *
 * 用于构建页面链表，将页面连接起来。
 * 设置后需要标记页面为脏页。
 *
 * @param page_id 下一个页面的页面号
 */
void PageHeader::SetNextPageId(pageid_t page_id) {
  *next_page_id_ = page_id;
  page_->SetDirty();
}

/**
 * LAB 2: 设置页面的 LSN
 *
 * 用于 WAL 日志，记录最后修改该页面的日志序列号。
 * 设置后需要标记页面为脏页。
 *
 * @param page_lsn 要设置的 LSN 值
 */
void PageHeader::SetPageLSN(lsn_t page_lsn) {
  *page_lsn_ = page_lsn;
  page_->SetDirty();
}

/**
 * 生成页面内容的字符串表示（用于调试）
 *
 * 输出页面的元数据（page_lsn、next_page_id、lower、upper）以及所有槽位的详细信息。
 * 会检测并报告页面结构错误（如 lower > upper、记录越界等）。
 *
 * @return 页面信息的字符串
 */
std::string PageHeader::ToString() const {
  std::ostringstream oss;
  oss << "TablePage[" << std::endl;
  oss << "  page_lsn: " << *page_lsn_ << std::endl;
  oss << "  next_page_id: " << *next_page_id_ << std::endl;
  oss << "  lower: " << *lower_ << std::endl;
  oss << "  upper: " << *upper_ << std::endl;
  if (*lower_ > *upper_) {
    oss << "\n***Error: lower > upper***" << std::endl;
  }
  oss << "  slots: " << std::endl;
  for (size_t i = 0; i < GetRecordCount(); i++) {
    oss << "    " << i << ": offset " << slots_[i].offset_ << ", size " << slots_[i].size_ << " ";
    if (slots_[i].size_ <= RECORD_HEADER_SIZE) {
      oss << "***Error: record size smaller than header size***" << std::endl;
    } else if (slots_[i].offset_ + RECORD_HEADER_SIZE >= DB_PAGE_SIZE) {
      oss << "***Error: record offset out of page boundary***" << std::endl;
    } else {
      RecordHeader header;
      header.DeserializeFrom(page_data_ + slots_[i].offset_);
      oss << header.ToString() << std::endl;
    }
  }
  oss << "]\n";
  return oss.str();
}

}  // namespace huadb
