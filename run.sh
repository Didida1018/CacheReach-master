#!/bin/bash
# 修复 Git LFS 误跟踪问题
# 只保留 data/*.gra 和 query/*.txt 使用 LFS，其余文件改为普通 Git 管理

set -e  # 遇到错误立即停止

echo "=== 1. 备份当前的 .gitattributes ==="
cp .gitattributes .gitattributes.backup 2>/dev/null || true

echo "=== 2. 重写 .gitattributes，只保留大文件规则 ==="
cat > .gitattributes << 'EOF'
# 大文件使用 Git LFS
data/*.gra filter=lfs diff=lfs merge=lfs -text
query/*.txt filter=lfs diff=lfs merge=lfs -text
EOF

echo "=== 3. 从 LFS 中移除所有不该跟踪的文件类型 ==="
git lfs untrack "*.cpp" 2>/dev/null || true
git lfs untrack "*.h" 2>/dev/null || true
git lfs untrack "*.sh" 2>/dev/null || true
git lfs untrack "makefile" 2>/dev/null || true
git lfs untrack "run.sh" 2>/dev/null || true
git lfs untrack "git.sh" 2>/dev/null || true
git lfs untrack ".gitattributes" 2>/dev/null || true

echo "=== 4. 将普通文件从 Git 索引中移除（保留工作区文件） ==="
git rm --cached -f \
  Graph.cpp Graph.h Util.cpp Util.h main.cpp \
  makefile git.sh run.sh 2>/dev/null || true

# 如果有其他代码文件，可以取消注释下一行并添加
# git rm --cached -f *.cpp *.h 2>/dev/null || true

echo "=== 5. 重新添加这些文件作为普通 Git 文件 ==="
git add Graph.cpp Graph.h Util.cpp Util.h main.cpp \
  makefile git.sh run.sh 2>/dev/null || true
git add .gitattributes

echo "=== 6. 提交更改 ==="
git commit -m "Fix LFS: only data/*.gra and query/*.txt use LFS; others are normal files"

echo "=== 7. 强制推送到 GitHub（覆盖远程历史） ==="
git push origin master --force

echo "=== 8. 验证 LFS 跟踪状态 ==="
git lfs ls-files

echo "=== 完成！==="
echo "现在访问 GitHub 仓库，应该可以看到 .cpp/.h/.sh 等文件直接显示源码，"
echo "而 .gra 和 .txt 大文件仍然由 LFS 管理（显示指针）。"