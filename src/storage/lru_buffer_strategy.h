//===----------------------------------------------------------------------===//
//
//                         HuaDB
//
// lru_buffer_strategy.h
//
// Identification: src/storage/lru_buffer_strategy.h
//
//===----------------------------------------------------------------------===//

#pragma once

#include "storage/buffer_strategy.h"

namespace huadb {

/**
 * LRUBufferStrategy 类实现了最近最少使用（Least Recently Used）缓存替换策略
 *
 * LRU 是一种广泛使用的缓存替换算法，其核心思想是：
 * "当需要淘汰页面时，选择最长时间未被访问的页面"
 *
 * 数据结构设计：
 * - 使用链表或类似结构维护页面的访问时间顺序
 * - 最近访问的页面放在链表前端，最久未访问的页面在链表后端
 * - Access() 操作将页面移到链表前端，表示最近访问
 * - Evict() 操作从链表后端选择页面进行淘汰
 *
 * 与 BufferPool 的关系：
 * - BufferStrategy 是 BufferPool 的策略接口
 * - BufferPool 调用 Access() 通知页面访问事件
 * - 当缓冲池满时，BufferPool 调用 Evict() 获取要淘汰的页面帧号
 */
class LRUBufferStrategy : public BufferStrategy {
 public:
  void Access(size_t frame_no) override;
  size_t Evict() override;
};

}  // namespace huadb
