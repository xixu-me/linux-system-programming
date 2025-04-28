#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#define FIFO_PATH "/tmp/collector_fifo"

int main() {
	int fd;
	int num;

	// 创建 FIFO 如果不存在
	if (mkfifo(FIFO_PATH, 0666) == -1 && errno != EEXIST) {
		perror("创建 FIFO 失败");
		exit(EXIT_FAILURE);
	}

	printf("Collector 已启动，正在连接 Calculator...\n");

	// 打开 FIFO 以写入
	fd = open(FIFO_PATH, O_WRONLY);
	if (fd == -1) {
		perror("打开 FIFO 失败");
		exit(EXIT_FAILURE);
	}

	printf("已连接！输入整数（Ctrl+D 退出）：\n");

	while (scanf("%d", &num) == 1) {
		if (write(fd, &num, sizeof(int)) == -1) {
			perror("写入错误");
			break;
		}
		printf("已发送：%d\n", num);
	}

	close(fd);
	printf("Collector 退出。\n");

	return 0;
}