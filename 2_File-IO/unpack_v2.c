#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdbool.h>

// 列出打包文件中的内容，不解包
void list_pack_contents(const char *packfile) {
	int pack_fd;

	pack_fd = open(packfile, O_RDONLY);
	if (pack_fd == -1) {
		perror("无法打开打包文件");
		exit(1);
	}

	printf("%-30s %s\n", "文件名", "文件大小 (字节)");
	printf("%-30s %s\n", "------------------------------", "---------------");

	while (1) {
		int name_len, file_size;

		// 读取文件名长度
		if (read(pack_fd, &name_len, sizeof(int)) <= 0) {
			break; // 到达文件末尾或出错
		}

		// 读取文件名
		char *filename = (char *)malloc(name_len + 1);
		if (filename == NULL) {
			perror("内存分配失败");
			close(pack_fd);
			exit(1);
		}

		read(pack_fd, filename, name_len);
		filename[name_len] = '\0'; // 添加字符串结束符

		// 读取文件大小
		read(pack_fd, &file_size, sizeof(int));

		// 打印文件信息
		printf("%-30s %d\n", filename, file_size);

		// 跳过文件数据
		lseek(pack_fd, file_size, SEEK_CUR);

		free(filename);
	}

	close(pack_fd);
}

// 解包文件
void extract_pack(const char *packfile) {
	int pack_fd, file_fd;
	char buffer[4096];
	ssize_t bytes_read;

	// 打开打包文件
	pack_fd = open(packfile, O_RDONLY);
	if (pack_fd == -1) {
		perror("无法打开打包文件");
		exit(1);
	}

	// 读取并解包文件
	while (1) {
		int name_len, file_size;

		// 读取文件名长度
		if (read(pack_fd, &name_len, sizeof(int)) <= 0) {
			break; // 到达文件末尾或出错
		}

		// 读取文件名
		char *filename = (char *)malloc(name_len + 1);
		if (filename == NULL) {
			perror("内存分配失败");
			close(pack_fd);
			exit(1);
		}

		read(pack_fd, filename, name_len);
		filename[name_len] = '\0'; // 添加字符串结束符

		// 读取文件大小
		read(pack_fd, &file_size, sizeof(int));

		printf("正在解包: %s (%d 字节)\n", filename, file_size);

		// 创建目标文件
		file_fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
		if (file_fd == -1) {
			perror("无法创建目标文件");
			free(filename);
			continue;
		}

		// 拷贝文件数据
		int remaining = file_size;
		while (remaining > 0) {
			int to_read = (remaining < sizeof(buffer)) ? remaining : sizeof(buffer);
			bytes_read = read(pack_fd, buffer, to_read);
			if (bytes_read <= 0) {
				perror("读取文件数据失败");
				break;
			}

			write(file_fd, buffer, bytes_read);
			remaining -= bytes_read;
		}

		close(file_fd);
		free(filename);
	}

	close(pack_fd);
	printf("解包完成\n");
}

int main(int argc, char *argv[]) {
	bool list_only = false;
	char *packfile = NULL;

	// 解析命令行参数
	if (argc == 2) {
		// 普通解包模式
		packfile = argv[1];
	}
	else if (argc == 3 && strcmp(argv[1], "-l") == 0) {
		// 列表模式
		list_only = true;
		packfile = argv[2];
	}
	else {
		fprintf(stderr, "用法:\n");
		fprintf(stderr, "  %s <打包文件>        - 解包文件\n", argv[0]);
		fprintf(stderr, "  %s -l <打包文件>     - 列出打包文件内容\n", argv[0]);
		return 1;
	}

	if (list_only) {
		list_pack_contents(packfile);
	}
	else {
		extract_pack(packfile);
	}

	return 0;
}
