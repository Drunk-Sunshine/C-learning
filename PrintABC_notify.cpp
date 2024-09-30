#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <string>
using namespace std;

class PrintABC {
private:
	int state;
	int times;
	mutex mtx;
	condition_variable cv;

public:
	PrintABC(int times):state(0), times(times){}

	void printLetter(const string& name, int targetState) {
		for (int i = 0; i < times; ++i) {
			unique_lock<mutex> lock(mtx);
			cv.wait(lock, [this, targetState]() {
				return state % 3 == targetState;
				});
			cout << name;
			state++;
			cv.notify_all();
		}
	}
};

int main() {
	PrintABC printABC(5);

	std::thread tA(&PrintABC::printLetter, &printABC, "A", 0);
	std::thread tB(&PrintABC::printLetter, &printABC, "B", 1);
	std::thread tC(&PrintABC::printLetter, &printABC, "C", 2);

	tA.join();
	tB.join();
	tC.join();

	return 0;
}
