#!/bin/bash
# 统计当前目录下的文件总数
file_count=$(ls | wc -l)
# 统计当前目录下的子目录数
dir_count=$(ls -l | grep '^d' | wc -l)
# 统计当前目录下有执行权限的普通文件数
exec_file_count=$(ls -l | grep '^-..x' | wc -l)
# 输出结果
echo "当前目录下的文件总数: $file_count"
echo "当前目录下的子目录数: $dir_count"
echo "当前目录下有执行权限的普通文件数: $exec_file_count"
