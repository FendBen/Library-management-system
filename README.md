# 图书管理系统（重构版）

一个结构良好、可维护的图书馆管理系统，功能覆盖**图书管理、读者管理、借书/还书、逾期判断与统计**，数据以 **JSON 文件**持久化。

> **重要说明**：原仓库 `FendBen/Library-management-system` 经核实**没有任何源码**（仅一个 `.gitignore`）。因此本工程并非对既有代码的逐行重构，而是**按照该项目的定位（“用 C++ 开发的简易图书馆管理系统”）从零实现的一版高质量替代实现**，符合课程设计要求。

## 功能清单

| 模块 | 功能 |
|---|---|
| 图书管理 | 添加、删除、修改、按书名/作者/ISBN/编号模糊搜索（忽略大小写） |
| 读者管理 | 添加、删除读者；有未归还图书时禁止删除 |
| 借阅管理 | 借书（校验库存、重复借阅）、还书、借期自动计算应还日期 |
| 逾期判断 | 未归还且超过应还日期自动标记“逾期” |
| 统计 | 图书种数、馆藏/可借册数、读者数、在借笔数、逾期笔数 |
| 持久化 | 所有变更即时写入 JSON 文件（默认 `data/library.json`） |

## 目录结构

```
Library-management-system-refactored/
├── CMakeLists.txt
├── include/
│   ├── model/          # 实体：Book / Reader / BorrowRecord
│   ├── storage/        # Storage 抽象 + JsonStorage 实现
│   ├── service/        # LibraryService 业务层
│   ├── cli/            # CliApp 交互层
│   └── util/           # 工具：Json / Date / Text
├── src/                # 与 include 对应的实现
├── tests/              # 单元测试（JSON、日期、业务、持久化）
└── data/               # 运行时数据目录
```

## 分层设计（重构要点）

```
CliApp（交互层）
   ↓
LibraryService（业务层：校验 + 规则 + 协调持久化）
   ↓
Storage（抽象接口）── JsonStorage（JSON 文件实现）
   ↓
util / model（工具与实体）
```

- **关注点分离**：UI、业务、存储、实体各自独立，可替换任一实现（如把 JSON 存储换成 SQLite 只需新增一个 `Storage` 子类）。
- **依赖抽象**：服务层只依赖 `Storage` 接口，便于测试与扩展。
- **错误处理**：所有失败操作返回明确原因（`err` 参数），不再用裸 `exit` 或静默失败。
- **数据安全**：文件损坏时抛异常并终止而非覆盖清空；删除有约束校验。
- **无第三方依赖**：JSON 解析/序列化、日期计算均为标准库实现，`cmake` 一键构建。

## 构建与运行

```bash
cmake -S . -B build
cmake --build build -j$(nproc)
./build/library_manager                 # 默认数据文件 data/library.json
./build/library_manager /path/to/x.json # 指定数据文件
```

## 运行测试

```bash
ctest --test-dir build --output-on-failure
```

## 数据文件格式（data/library.json）

```json
{
  "books":   [ { "id": "B001", "title": "三体", "author": "刘慈欣", "isbn": "", "totalCopies": 3, "availableCopies": 2 } ],
  "readers": [ { "id": "R001", "name": "苏贝力", "phone": "" } ],
  "records": [ { "id": "BR001", "bookId": "B001", "readerId": "R001",
                 "borrowDate": "2026-10-04", "dueDate": "2026-10-18", "returnDate": "" } ]
}
```

## 后续计划

- 同步 iOS 版应用（`Library-iOS/` 目录，功能与本项目一致）
- 可扩展：多条件组合检索、借阅历史统计报表、SQLite 存储后端
