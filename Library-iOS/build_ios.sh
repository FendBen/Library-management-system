#!/usr/bin/env bash
# ============================================================
# 在 Linux 上交叉编译 iOS 版「图书管理」应用
# 产物：Payload/LibraryManager.app（未签名）+ LibraryManager-unsigned.ipa
# 真机安装需要签名，见 sign_ipa.sh
# 依赖：clang-14+ / ld64.lld-14（apt install clang lld） / iOS SDK
# ============================================================
set -euo pipefail

ROOT="$(cd "$(dirname "$0")" && pwd)"
SDK="${IOS_SDK:-$ROOT/../.ios-toolchain/sdks/iPhoneOS15.6.sdk}"
APP="$ROOT/Payload/LibraryManager.app"
OBJDIR="$ROOT/build-obj"
CLANG="${CLANG:-clang}"
TARGET="arm64-apple-ios13.0"

if [ ! -d "$SDK" ]; then
  echo "错误：找不到 iOS SDK（$SDK）。请先下载解压 theos/sdks 的 iPhoneOS15.6.sdk。"
  exit 1
fi

SOURCES="main.m AppDelegate.m LBBook.m LBReader.m LBRecord.m LBLibraryStore.m \
BooksListViewController.m ReadersListViewController.m BorrowViewController.m"

rm -rf "$OBJDIR" "$ROOT/Payload"
mkdir -p "$OBJDIR" "$APP"

OBJS=()
for src in $SOURCES; do
  echo "== 编译 $src"
  "$CLANG" -target "$TARGET" -isysroot "$SDK" -fobjc-arc -mios-version-min=13.0 \
    -I"$ROOT" -Wno-everything \
    -c "$ROOT/$src" -o "$OBJDIR/$src.o"
  OBJS+=("$OBJDIR/$src.o")
done

echo "== 链接"
"$CLANG" -target "$TARGET" -isysroot "$SDK" -fuse-ld=lld \
  "${OBJS[@]}" -o "$APP/LibraryManager" \
  -framework UIKit -framework Foundation -framework CoreGraphics -lobjc -lc

cp "$ROOT/Info.plist" "$APP/Info.plist"

echo "== 打包未签名 IPA"
cd "$ROOT"
rm -f LibraryManager-unsigned.ipa
zip -qry LibraryManager-unsigned.ipa Payload

echo ""
echo "完成：$ROOT/LibraryManager-unsigned.ipa"
echo "（未签名，无法直接安装；签名请执行 ./sign_ipa.sh）"
