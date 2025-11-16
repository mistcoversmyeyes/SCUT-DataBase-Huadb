//===----------------------------------------------------------------------===//
//
//                         HuaDB
//
// lru_buffer_strategy.cpp
//
// Identification: src/storage/lru_buffer_strategy.cpp
//
//===----------------------------------------------------------------------===//

#include "storage/lru_buffer_strategy.h"

namespace huadb {

/**
 * 处理缓存页面访问事件
 *
 * 实现逻辑：
 * 1. 将被访问的页面标记为最近使用
 * 2. 更新 LRU 顺序，确保该页面不会被过早淘汰
 * 3. 如果页面已在 LRU 结构中，将其移动到最前面
 * 4. 如果页面是新加入的，将其添加到最前面
 *
 * @param frame_no 被访问页面的帧号
 *
 * LAB 1 实现要点：
 * - 可选择的数据结构：std::list（双向链表）配合 std::unordered_set/frame_no 映射
 * - O(1) 实现：使用链表 + 哈希表的组合数据结构
 * - 简单实现：直接使用 std::list，O(n) 查找但逻辑简单
 * - 处理重复访问：如果页面已在链表中，先删除再添加到前端
 */
void LRUBufferStrategy::Access(size_t frame_no) {
  // LAB 1: 实现 LRU 访问逻辑
  // 1. 维护一个数据结构来记录页面的访问顺序
  // 2. 将被访问的页面移到最前端（最近使用）
  // 3. 如果页面已存在，先移除旧位置再添加到前端
  // 可选数据结构：
  // - std::list<size_t>：简单的双向链表，O(n) 查找
  // - std::list + std::unordered_set：O(1) 查找和更新
  // LAB 1 BEGIN
  auto it = frames.find(frame_no);

  if (it != frames.end()) {
    lru_.erase(it->second);
  }

  lru_.push_front(frame_no);

  frames[frame_no] = lru_.begin();
  return;
};

/**
 * 选择并淘汰最久未使用的页面
 *
 * 实现逻辑：
 * 1. 从 LRU 数据结构的末端选择页面
 * 2. 该页面是最长时间未被访问的页面
 * 3. 从 LRU 结构中移除该页面
 * 4. 返回要淘汰的页面帧号
 *
 * @return 应该淘汰的页面帧号
 *
 * LAB 1 实现要点：
 * - 检查 LRU 结构是否为空，空则返回默认值
 * - 返回链表最后一个元素的帧号（最久未使用）
 * - 确保返回的帧号在有效范围内
 * - 注意边界情况：缓冲池为空时的处理
 */
size_t LRUBufferStrategy::Evict() {
  // LAB 1: 实现 LRU 淘汰逻辑
  // 1. 从 LRU 结构的末端选择最久未使用的页面
  // 2. 返回该页面的帧号
  // 3. 从 LRU 结构中移除该页面
  // 4. 处理空缓冲池的特殊情况
  // LAB 1 BEGIN

  if (lru_.empty()){
    return 0;
  }

  auto frame_no = lru_.back();
  lru_.pop_back();
  frames.erase(frame_no);

  return frame_no;
}

}  // namespace huadb
