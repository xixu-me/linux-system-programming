#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

int main() {
	int pipe_fd[2];
	pid_t pid;
	int num, sum = 0;

	// 创建管道
	if (pipe(pipe_fd) == -1) {
		perror("管道创建失败");
		exit(EXIT_FAILURE);
	}

	// 创建子进程
	pid = fork();

	if (pid < 0) {
		// fork失败
		perror("fork 失败");
		exit(EXIT_FAILURE);
	}
	else if (pid == 0) {
		// 子进程 - Calculator
		close(pipe_fd[1]); // 关闭管道写端

		printf("[Calculator] 启动。等待整数...\n");

		while (read(pipe_fd[0], &num, sizeof(int)) > 0) {
			sum += num;
			printf("[Calculator] 收到：%d，当前总和：%d\n", num, sum);
		}

		close(pipe_fd[0]);
		printf("[Calculator] 退出。\n");
		exit(EXIT_SUCCESS);
	}
	else {
		// 父进程 - Collector
		close(pipe_fd[0]); // 关闭管道读端

		printf("[Collector] 启动。输入整数（Ctrl+D 退出）：\n");

		while (scanf("%d", &num) == 1) {
			if (write(pipe_fd[1], &num, sizeof(int)) == -1) {
				perror("写入错误");
				break;
			}
			printf("[Collector] 已发送：%d\n", num);
		}

		close(pipe_fd[1]); // 关闭管道写端以向子进程发送EOF信号
		printf("[Collector] 等待 Calculator 完成...\n");

		// 等待子进程完成
		wait(NULL);

		printf("[Collector] 退出。\n");
	}

	return 0;
}