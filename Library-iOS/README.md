# 图书管理（iOS 版）

把「图书管理系统」移植为 **iOS 原生应用**（Objective-C + UIKit），功能与重构后的 C++ 项目一致：图书增删改查、读者管理、借书/还书、逾期标记、统计概览，数据以 JSON 持久化到 App 沙盒 `Documents/library.json`。

```
Library-iOS/
├── AppDelegate.m / main.m          # 入口：三 Tab（图书 / 读者 / 借阅）
├── LBBook.h/m  LBReader.h/m  LBRecord.h/m   # 数据模型
├── LBLibraryStore.h/m             # 数据层 + 业务规则（JSON 持久化）
├── BooksListViewController.m      # 图书列表：搜索 / 新增 / 修改 / 删除
├── ReadersListViewController.m    # 读者列表：新增 / 删除
├── BorrowViewController.m         # 借阅：选读者选书借书、还书、记录、统计
├── Info.plist                     # BundleID: com.fendben.librarymanager
├── build_ios.sh                   # Linux 交叉编译 → 未签名 IPA
├── sign_ipa.sh                    # P12 证书 + 描述文件签名 → 可安装 IPA
└── README.md
```

## 为什么不能直接给出「可用 IPA」

在**没有 macOS + Xcode** 的环境里无法产出可安装的签名 IPA，原因：

1. **代码签名**：iOS 要求每个 App 由 Apple 认可的证书签名。签名工具链（codesign）随 Xcode 分发，且需要你的 **P12 证书（含私钥）+ 与设备 UDID 绑定的描述文件（.mobileprovision）**。
2. **未签名的 IPA** 在正常 iPhone 上无法安装（越狱除外）。

因此本目录提供的是：**编译好的未签名 IPA + 一键签名脚本**。你已持有证书，只要补上描述文件和设备 UDID，即可在任意电脑（Windows/Linux）上完成签名，产出真正可安装的 IPA。

## 在 Linux 上重新构建（未签名 IPA）

```bash
sudo apt install -y clang lld          # 交叉编译工具链
# 下载 iOS SDK 到 .ios-toolchain/sdks/iPhoneOS15.6.sdk（已随工程提供路径）
./build_ios.sh
# 产物：LibraryManager-unsigned.ipa
```

## 签名并产出可安装 IPA

```bash
# 1) 准备材料（在 Apple Developer 网页端即可完成，无需 Mac）：
#    - P12 证书：用 OpenSSL 生成 CSR → 开发者后台创建证书 → 导出 .p12（含私钥）
#    - 描述文件：开发者后台创建 Ad-Hoc / Development 描述文件，
#      选择你的证书 + 注册你的 iPhone UDID，Bundle ID 填 com.fendben.librarymanager
# 2) 执行签名：
./sign_ipa.sh 证书.p12 证书密码 描述文件.mobileprovision
# 产物：LibraryManager-signed.ipa
```

## 安装签名后的 IPA

| 设备 | 方式 |
|---|---|
| Windows 电脑 | iTunes（把 IPA 拖入设备）/ 3uTools / Sideloadly |
| Mac 电脑 | Apple Configurator 2 或 Finder 直接拖入 |
| 说明 | 描述文件必须包含你的设备 UDID，且设备信任该开发者证书 |

## 与 C++ 版的一致性

- 相同的数据模型字段与 JSON 格式（`books/readers/records`）
- 相同的业务规则：重复借阅拦截、库存校验、借出中不可删书/删读者、逾期计算
- C++ 版的数据文件可直接迁移到 iOS 沙盒使用（字段一一对应）
