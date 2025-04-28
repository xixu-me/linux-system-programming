#!/bin/bash
# 清空输出文件
>record2
# 从 record 文件中读取每一行记录
while read name ch math eng; do
    # 判断三个科目中是否有不及格（成绩低于 60）
    if [ "$ch" -lt 60 ] || [ "$math" -lt 60 ] || [ "$eng" -lt 60 ]; then
        echo "$name $ch $math $eng" >>record2
    fi
done <record
