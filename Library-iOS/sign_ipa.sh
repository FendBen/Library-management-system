#!/usr/bin/env bash
# ============================================================
# 用 P12 证书 + 描述文件给未签名 IPA 签名，产出可安装的 IPA
#
# 用法：
#   ./sign_ipa.sh 证书.p12 证书密码 描述文件.mobileprovision [BundleID]
#
# 示例：
#   ./sign_ipa.sh ~/certs/dist.p12 "mypass" ~/certs/DevProvision.mobileprovision
#
# 说明：
#   - BundleID 默认 com.fendben.librarymanager；若描述文件绑定的
#     BundleID 不同，请用第 4 个参数覆盖
#   - 需要 zsign（本脚本会自动尝试编译安装到 .ios-toolchain/zsign）
#   - 签名后可用 Windows 端 iTunes/3uTools/Sideloadly 或 Apple Configurator 安装
# ============================================================
set -euo pipefail

P12="${1:?用法: ./sign_ipa.sh 证书.p12 证书密码 描述文件.mobileprovision [BundleID]}"
PASS="$2"
PROV="$3"
BID="${4:-com.fendben.librarymanager}"

ROOT="$(cd "$(dirname "$0")" && pwd)"
TOOLCHAIN="$ROOT/../.ios-toolchain"
ZSIGN_DIR="$TOOLCHAIN/zsign-src"
ZSIGN="$TOOLCHAIN/bin/zsign"
UIPA="$ROOT/LibraryManager-unsigned.ipa"
OPA="$ROOT/LibraryManager-signed.ipa"

# ---- 准备 zsign ----
if [ ! -x "$ZSIGN" ]; then
  echo "== 编译安装 zsign"
  if [ ! -d "$ZSIGN_DIR" ]; then
    git clone --depth 1 https://github.com/zhlynn/zsign "$ZSIGN_DIR"
  fi
  mkdir -p "$TOOLCHAIN/bin"
  (cd "$ZSIGN_DIR/build/linux" && make -j"$(nproc)")
  cp "$ZSIGN_DIR/bin/zsign" "$ZSIGN"
fi

rm -f "$OPA"

# ---- 签名（zsign 直接处理 IPA：注入描述文件 + 用 P12 重签全部二进制）----
"$ZSIGN" -k "$P12" -p "$PASS" -m "$PROV" -b "$BID" -o "$OPA" -i "$UIPA"

echo ""
echo "签名完成：$OPA"
echo "安装方式："
echo "  1) Windows：安装 iTunes（或 3uTools / Sideloadly），连接 iPhone 后把 IPA 拖入设备"
echo "  2) Mac：Apple Configurator 2 或 Finder（双击/拖入 iPhone）"
echo "  3) 描述文件必须是 Ad-Hoc/Development 类型且包含你的设备 UDID"
