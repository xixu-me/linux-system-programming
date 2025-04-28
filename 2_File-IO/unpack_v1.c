#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

int main() {
	int pack_fd, file_fd;
	char buffer[4096];
	ssize_t bytes_read;

	// 打开打包文件
	pack_fd = open("a.pack", O_RDONLY);
	if (pack_fd == -1) {
		perror("无法打开打包文件");
		return 1;
	}

	// 读取并解包文件
	while (1) {
		int name_len, file_size;

		// 读取文件名长度
		if (read(pack_fd, &name_len, sizeof(int)) <= 0) {
			// 到达文件末尾或出错
			break;
		}

		// 读取文件名
		char *filename = (char *)malloc(name_len + 1);
		if (filename == NULL) {
			perror("内存分配失败");
			close(pack_fd);
			return 1;
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
	return 0;
}
