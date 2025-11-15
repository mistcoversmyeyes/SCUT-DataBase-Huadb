//===----------------------------------------------------------------------===//
//
//                         HuaDB
//
// table_scan.cpp
//
// Identification: src/table/table_scan.cpp
//
//===----------------------------------------------------------------------===//

#include "table/table_scan.h"

#include "table/page_header.h"

namespace huadb {

/**
 * 构造表扫描器
 *
 * 初始化表扫描器，设置缓冲池引用、表对象和起始扫描位置。
 * 扫描将从指定的 RID 开始，逐条返回记录。
 *
 * @param buffer_pool 缓冲池引用，用于访问页面数据
 * @param table 要扫描的表对象
 * @param rid 扫描起始位置（行标识符）
 */
TableScan::TableScan(BufferPool &buffer_pool, std::shared_ptr<Table> table, Rid rid)
    : buffer_pool_(buffer_pool), table_(std::move(table)), rid_(rid) {}

/**
 * 获取下一条可见记录
 *
 * 实现表的核心扫描逻辑，支持事务隔离级别和 MVCC。
 * 扫描器会维护当前扫描位置（rid_），每次调用返回下一条符合条件的记录。
 *
 * 扫描流程：
 * 1. 检查是否到达表末尾（rid_.page_id_ 为 NULL_PAGE_ID）
 * 2. 获取当前页面并查找指定 slot_id 指向的记录
 * 3. 根据事务隔离级别和活跃事务集合判断记录可见性（LAB 3）
 * 4. 如果当前页面扫描完毕，移动到下一个页面继续扫描
 * 5. 更新 rid_ 指向下一条记录的位置
 * 6. 返回可见的记录，扫描结束时返回 nullptr
 *
 * 事务可见性判断（LAB 3）：
 * - READ_UNCOMMITTED：读取最新版本，不考虑其他事务
 * - READ_COMMITTED：只读取已提交事务的版本
 * - REPEATABLE_READ：确保事务内可重复读，处理 MVCC
 * - SERIALIZABLE：最高隔离级别，防止幻读
 *
 * @param xid 事务 ID（LAB 3），用于 MVCC 和可见性判断
 * @param isolation_level 事务隔离级别（LAB 3），决定记录可见性规则
 * @param cid 命令 ID（LAB 3），事务内部的操作序列号
 * @param active_xids 当前活跃的事务 ID 集合（LAB 3），用于 MVCC 可见性判断
 * @return 下一条可见记录的共享指针，扫描结束时返回 nullptr
 *
 * LAB 1 实现要点：
 * - 处理空表扫描（rid_.page_id_ == NULL_PAGE_ID）
 * - 使用 buffer_pool_.GetPage() 获取页面数据
 * - 使用 PageHeader 作为页面代理访问记录
 * - 正确处理页面链表遍历
 *
 * LAB 3 实现要点：
 * - 实现 MVCC 可见性判断逻辑
 * - 根据隔离级别过滤不可见记录
 * - 处理事务内的命令 ID（cid）比较
 * - 考虑活跃事务集合的影响
 */
std::shared_ptr<Record> TableScan::GetNextRecord(xid_t xid, IsolationLevel isolation_level, cid_t cid,
                                                 const std::unordered_set<xid_t> &active_xids) {
  // 根据事务隔离级别及活跃事务集合，判断记录是否可见
  // LAB 3 BEGIN

  // 每次调用读取一条记录
  // 读取时更新 rid_ 变量，避免重复读取
  // 扫描结束时，返回空指针
  // 注意处理扫描空表的情况（rid_.page_id_ 为 NULL_PAGE_ID）
  // LAB 1 BEGIN
  return nullptr;
}

}  // namespace huadb
