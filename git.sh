# --- 1. 环境清理与代理配置 (针对 v2rayN) ---
echo "正在配置 Git 代理为 v2rayN 默认端口 10809..."
git config --global http.proxy http://127.0.0.1:10809
git config --global https.proxy http://127.0.0.1:10809

# --- 2. 初始化与 LFS 修复 ---
echo "初始化仓库并优化 LFS 配置..."
git init
git lfs install

git lfs track *

# --- 3. 提交文件 ---
echo "正在添加文件并提交..."
git add .gitattributes
git add .
git commit -m "chore: 修复代理与LFS配置，初始化项目代码"

# --- 4. 设置远程仓库 ---
# 如果已经添加过 origin，先删除再添加
git remote remove origin 2>/dev/null
git remote add origin https://github.com/Didida1018/Reachability.git

# --- 5. 推送 ---
echo "正在强制推送至 GitHub..."
# 使用 -f 解决 "failed to push some refs" 历史冲突问题
git push origin master -f

echo "操作完成！请检查 GitHub 仓库。"