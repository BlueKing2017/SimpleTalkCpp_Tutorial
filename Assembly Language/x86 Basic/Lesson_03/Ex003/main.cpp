#include "my_common.h"

#include <xmmintrin.h>

class alignas(16) MyVec4 {
public:
	float x, y, z, w;

	void set(float x_, float y_, float z_, float w_) {
		x = x_;
		y = y_;
		z = z_;
		w = w_;
	}

	void print() {
		printf("Vec4<%f, %f, %f, %f>\n", x, y ,z, w);
	}
};

const int N = 1000000;
const int M = 100;

void add_cpp(MyVec4* dst, MyVec4* a, MyVec4* b, int size) {
	for (int i = 0; i < size; i++) {
		dst->x = a->x + b->x;
		dst->y = a->y + b->y;
		dst->z = a->z + b->z;
		dst->w = a->w + b->w;

		dst++;
		a++;
		b++;
	}
}

extern "C" void asm_add_cpp(MyVec4* dst, MyVec4* a, MyVec4* b, int size);

void test_add() {
	std::vector<MyVec4> a;
	std::vector<MyVec4> b;
	std::vector<MyVec4> dst;

	a.resize(N);
	b.resize(N);
	dst.resize(N);

	for (int i = 0; i < N; i++) {
		float f = static_cast<float>(i);
		a[i].set(f + 1.0f,
				 f + 2.0f,
				 f + 3.0f,
				 f + 4.0f);

		b[i].set(f + 5.0f,
				 f + 6.0f,
				 f + 7.0f,
				 f + 8.0f);
	}

	{
		printf("\n-- add_cpp --\n");
		MyTimer timer;
		for (int j = 0; j < M; j++) {
			add_cpp(dst.data(), a.data(), b.data(), N);
		}
		timer.print();
		dst[10].print();
	}
	
	{
		printf("\n-- asm_add_cpp --\n");
		MyTimer timer;
		for (int j = 0; j < M; j++) {
			//printf("%p %p %p\n", dst.data(), a.data(), b.data());
			//printf("%p %p %p\n", dst.data() + N, a.data() + N, b.data() + N);
			asm_add_cpp(dst.data(), a.data(), b.data(), N);
		}
		timer.print();
		dst[10].print();
	}

	{
		printf("\n-- add_cpp --\n");
		MyTimer timer;
		for (int j = 0; j < M; j++) {
			add_cpp(dst.data(), a.data(), b.data(), N);
		}
		timer.print();
		dst[10].print();
	}
}

int main() {
	test_add();

	printf("\n===== Program Ended =====\n  Press any key to exit !!\n");
	_getch();
	return 0;
}

