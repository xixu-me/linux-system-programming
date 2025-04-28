#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define FIFO_PATH "/tmp/collector_fifo"

int main() {
	int fd;
	int num;
	int sum = 0;

	printf("Calculator 已启动，等待 Collector...\n");

	// 打开 FIFO 以读取
	fd = open(FIFO_PATH, O_RDONLY);
	if (fd == -1) {
		perror("打开 FIFO 失败");
		exit(EXIT_FAILURE);
	}

	printf("已连接！准备接收整数。\n");

	// 从 FIFO 读取整数并计算总和
	while (read(fd, &num, sizeof(int)) > 0) {
		sum += num;
		printf("接收到：%d，当前总和：%d\n", num, sum);
	}

	close(fd);
	printf("Calculator 退出。\n");

	return 0;
}