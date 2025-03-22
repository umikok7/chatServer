set -e  # 出错时立即退出
set -x  # 显示执行的命令

# 确保 build 目录存在
mkdir -p $(pwd)/build

# 清理并进入 build 目录
rm -rf $(pwd)/build/*
cd $(pwd)/build || exit 1

# 构建项目
cmake .. || exit 1
make || exit 1

echo "构建成功完成!"