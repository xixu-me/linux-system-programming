#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

int main(int argc, char *argv[]) {
	int pipefd[2]; // 管道文件描述符
	pid_t pid;
	int status;
	char buf[1024];
	long long sum = 0;
	long long num;

	// 获取输入n
	int n;
	if (argc > 1) {
		n = atoi(argv[1]);
	}
	else {
		printf("请输入一个整数: ");
		scanf("%d", &n);
	}

	// 创建管道
	if (pipe(pipefd) == -1) {
		perror("pipe");
		exit(EXIT_FAILURE);
	}

	// 创建子进程
	pid = fork();

	if (pid < 0) { // 创建子进程失败
		perror("fork");
		exit(EXIT_FAILURE);
	}
	else if (pid == 0) { // 子进程
		// 关闭管道读端
		close(pipefd[0]);

		// 将标准输出重定向到管道写端
		dup2(pipefd[1], STDOUT_FILENO);
		close(pipefd[1]);

		// 执行fabo程序，传入参数n
		char n_str[20];
		sprintf(n_str, "%d", n);
		execl("./fabo", "fabo", n_str, NULL);

		// 如果execl失败，则执行下面的代码
		perror("execl");
		exit(EXIT_FAILURE);
	}
	else { // 父进程
		// 关闭管道写端
		close(pipefd[1]);

		// 从管道读取子进程输出的每个斐波那契数，并计算总和
		FILE *fp = fdopen(pipefd[0], "r");
		if (fp == NULL) {
			perror("fdopen");
			exit(EXIT_FAILURE);
		}

		while (fgets(buf, sizeof(buf), fp) != NULL) {
			num = strtoll(buf, NULL, 10);
			sum += num;
		}

		// 关闭文件和管道读端
		fclose(fp);

		// 等待子进程结束
		waitpid(pid, &status, 0);

		// 打印总和
		printf("斐波那契数列前%d项的总和为: %lld\n", n, sum);
	}

	return 0;
}
