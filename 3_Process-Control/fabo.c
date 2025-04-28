#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
	int n, i;
	long long a = 0, b = 1, c;

	// 从命令行参数或键盘获取n
	if (argc > 1) {
		n = atoi(argv[1]);
	}
	else {
		printf("请输入一个整数: ");
		scanf("%d", &n);
	}

	// 验证输入
	if (n <= 0) {
		printf("请输入一个正整数\n");
		return 1;
	}

	// 生成并输出斐波那契数列的前n项
	printf("%lld\n", a); // 第1项: 0
	if (n > 1) {
		printf("%lld\n", b); // 第2项: 1
	}

	for (i = 3; i <= n; i++) {
		c = a + b;
		printf("%lld\n", c);
		a = b;
		b = c;
	}

	return 0;
}
