#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

volatile int number = 0;	// 当前偶数
volatile int direction = 1; // 1 表示递增，-1 表示递减

void handle_sigint(int sig) {
	// 按下 Ctrl+C 时改变方向
	direction = -direction;
	printf("\n方向已切换为 %s\n",
		(direction > 0) ? "递增" : "递减");
	// 重新注册信号处理函数
	signal(SIGINT, handle_sigint);
}

int main() {
	// 注册 SIGINT 信号处理函数
	signal(SIGINT, handle_sigint);

	printf("程序启动：输出偶数\n");
	printf("按下 Ctrl+C 可切换方向\n");

	while (1) {
		printf("%d\n", number);
		number += 2 * direction; // 根据方向加或减 2
		fflush(stdout);			 // 强制立即显示输出
		sleep(1);				 // 等待 1 秒
	}

	return 0;
}