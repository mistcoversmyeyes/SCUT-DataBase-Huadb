//===----------------------------------------------------------------------===//
//
//                         HuaDB
//
// table_scan.h
//
// Identification: src/table/table_scan.h
//
//===----------------------------------------------------------------------===//

#pragma once

#include <unordered_map>

#include "common/types.h"
#include "storage/buffer_pool.h"
#include "table/record.h"
#include "table/table.h"

namespace huadb {

/**
 * TableScan 类提供表的顺序扫描功能
 *
 * TableScan 实现了对表中记录的迭代访问，支持事务隔离级别控制。
 * 扫描从指定的 RID 开始，逐条返回表中的记录，直到表末尾。
 *
 * 扫描过程中会处理以下逻辑：
 * - 跨页面扫描：当一个页面扫描完毕后，自动切换到下一个页面
 * - 事务可见性：根据事务隔离级别和活跃事务集合判断记录是否可见（LAB 3）
 * - MVCC 支持：处理多版本并发控制下的记录可见性（LAB 3）
 *
 * TableScan 是表扫描器的基类，提供了记录扫描的核心接口。
 */
class TableScan {
 public:
  TableScan(BufferPool &buffer_pool, std::shared_ptr<Table> table, Rid rid);
  std::shared_ptr<Record> GetNextRecord(xid_t xid = NULL_XID, IsolationLevel isolation_level = DEFAULT_ISOLATION_LEVEL,
                                        cid_t cid = NULL_CID, const std::unordered_set<xid_t> &active_xids = {});

 private:
  BufferPool &buffer_pool_;                    // 缓冲池引用，用于访问页面数据
  std::shared_ptr<Table> table_;               // 要扫描的表对象
  Rid rid_;                                    // 当前扫描到的记录的 RID（行标识符）
};

}  // namespace huadb
