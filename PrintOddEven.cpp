#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <string>
using namespace std;

class OddEventPrint {
public:
	mutex mtx;
	condition_variable cv;
	const int limit;
	int count;
public:
	OddEventPrint(int initCount, int times): count(initCount),limit(times){}

	void print() {
		while (count < limit) {
			unique_lock<mutex> lock(mtx);
			cout << "线程[" << std::this_thread::get_id() << "]打印数字: " << ++count << std::endl;

			cv.notify_all();
			cv.wait(lock);
		}
		cv.notify_all();
	}
};

int main() {
	OddEventPrint printer(0, 10);
	thread oddThread(
		[&printer]() {
			while (printer.count < printer.limit) {
				printer.print();
			}
		}
	);
	thread evenThread(
		[&printer]() {
			while (printer.count < printer.limit) {
				printer.print();
			}
		}
	);
	oddThread.join();
	evenThread.join();

	return 0;
}