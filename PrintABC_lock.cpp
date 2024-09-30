// 用state和锁共同实现
#include <iostream>
#include <thread>
#include <mutex>
#include <atomic>

using namespace std;

class PrintABC {
private:
	int times; // 控制打印次数
	atomic<int> state{ 0 }; // 当前状态值，保证交替打印
	mutex _lock;

public:
	PrintABC(int times) : times(times) {}

	void printLetter(const string& name, int targetNum) {
		for (int i = 0; i < times;) {
			{
				unique_lock<mutex> lock(_lock);
				if (state % 3 == targetNum) {
					state++;
					i++;
					cout << name;
				}
			}
		}
	}
};

int main() {
	PrintABC loopthread(5);

	thread tA(&PrintABC::printLetter, &loopthread, "A", 0);
	thread tB(&PrintABC::printLetter, &loopthread, "B", 1); // 线程 B  
	thread tC(&PrintABC::printLetter, &loopthread, "C", 2); // 线程 C

	tA.join();
	tB.join();
	tC.join();

	return 0;
}