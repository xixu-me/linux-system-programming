#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int main() {
	DIR *dir;
	struct dirent *entry;
	int pack_fd, file_fd;
	char buffer[4096];
	ssize_t bytes_read;

	// 打开当前目录
	dir = opendir(".");
	if (dir == NULL) {
		perror("无法打开当前目录");
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
		// 忽略目录项和a.pack文件本身
		if (entry->d_type == DT_DIR || strcmp(entry->d_name, "a.pack") == 0) {
			continue;
		}

		printf("正在打包: %s\n", entry->d_name);

		// 写入文件名长度
		int name_len = strlen(entry->d_name);
		write(pack_fd, &name_len, sizeof(int));

		// 写入文件名
		write(pack_fd, entry->d_name, name_len);

		// 打开源文件
		file_fd = open(entry->d_name, O_RDONLY);
		if (file_fd == -1) {
			perror("无法打开源文件");
			continue;
		}

		// 获取文件大小
		struct stat st;
		fstat(file_fd, &st);
		int file_size = st.st_size;

		// 写入文件大小
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
