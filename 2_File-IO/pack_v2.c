#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
	DIR *dir;
	struct dirent *entry;
	int pack_fd, file_fd;
	char buffer[4096];
	ssize_t bytes_read;
	char filepath[PATH_MAX];

	// 检查命令行参数
	if (argc != 2) {
		fprintf(stderr, "用法: %s <目录名>\n", argv[0]);
		return 1;
	}

	// 打开指定目录
	dir = opendir(argv[1]);
	if (dir == NULL) {
		perror("无法打开指定目录");
		return 1;
	}

	// 创建打包文件
	pack_fd = open("a.pack", O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (pack_fd == -1) {
		perror("无法创建打包文件");
		closedir(dir);
		return 1;
	}

	// 遍历目录中的所有文件
	while ((entry = readdir(dir)) != NULL) {
		struct stat st;

		// 跳过 . 和 .. 目录
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
			continue;
		}

		// 构建完整的文件路径
		snprintf(filepath, PATH_MAX, "%s/%s", argv[1], entry->d_name);

		// 获取文件状态
		if (stat(filepath, &st) == -1) {
			perror("获取文件状态失败");
			continue;
		}

		// 只处理普通文件
		if (!S_ISREG(st.st_mode)) {
			continue;
		}

		printf("正在打包: %s\n", entry->d_name);

		// 写入文件名长度
		int name_len = strlen(entry->d_name);
		write(pack_fd, &name_len, sizeof(int));

		// 写入文件名
		write(pack_fd, entry->d_name, name_len);

		// 打开源文件
		file_fd = open(filepath, O_RDONLY);
		if (file_fd == -1) {
			perror("无法打开源文件");
			continue;
		}

		// 写入文件大小
		int file_size = st.st_size;
		write(pack_fd, &file_size, sizeof(int));

		// 写入文件数据
		while ((bytes_read = read(file_fd, buffer, sizeof(buffer))) > 0) {
			write(pack_fd, buffer, bytes_read);
		}

		close(file_fd);
	}

	close(pack_fd);
	closedir(dir);

	printf("打包完成，所有文件已打包到 a.pack\n");
	return 0;
}
