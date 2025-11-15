add_library(
  table
  OBJECT
  record_header.cpp
  record.cpp
  page_header.cpp
  table_scan.cpp
  table.cpp
)

set(ALL_OBJECT_FILES
  ${ALL_OBJECT_FILES} $<TARGET_OBJECTS:table>
  PARENT_SCOPE)
