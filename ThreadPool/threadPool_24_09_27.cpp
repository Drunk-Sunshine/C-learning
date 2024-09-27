#include <vector>
#include <queue>
#include <future>
#include <atomic>
#include <functional>
#include <stdexcept>

#include <iostream>

using namespace std;
#define ThreadMAX 16

class Threadpool {
private:
	using Task = function<void()>;
	vector<thread> _pools;
	queue<Task> _tasks;
	mutex _locks;
	condition_variable _cv;
	atomic<int> threadNum{ 0 };
	atomic<bool> _run{ true };

public:
	Threadpool(size_t num = 4) {
		addThread(num);
	}

	~Threadpool() {
		_run = false;
		_cv.notify_all();
		for (auto& t : _pools) {
			if (t.joinable()) {
				t.join();
			}
		}
	}

	template<class F, class... Args>
	auto commit(F&& f, Args&& ...args) -> future<decltype(f(args...))> {
		if (!_run) {
			throw runtime_error("The Pool is closed");
		}

		using return_type = decltype(f(args...));
		auto task = make_shared<packaged_task<return_type()>>(
			bind(forward<F>(f), forward<Args>(args)...)
		);

		future<return_type> future = task->get_future();
		{
			unique_lock<mutex> lock{ _locks };
			_tasks.emplace(
				[task] {
					(*task)();
				}
			);
		}
		_cv.notify_one();
		return future;

	}

	void addThread(size_t num) {
		for (; _pools.size() < ThreadMAX && num > 0; --num) {
			_pools.emplace_back(
				[this] {
					while (_run) {
						Task task;
						{
							unique_lock<mutex> lock{ _locks };
							_cv.wait(lock, [this] {
								return !_run || !_tasks.empty();
								});
							if (!_run && _tasks.empty()) {
								return;
							}
							task = move(_tasks.front());
							_tasks.pop();
						}
						threadNum--;
						task();
						threadNum++;
					}
				}
			);
			threadNum++;
		}
	}
};

mutex printlock;
int sum = 0;
void test() {
	{
		unique_lock<mutex> lock(printlock);
		sum++;
		cout << "the thread id is" << this_thread::get_id() << "; the sum is" << sum << endl;
	}
	this_thread::sleep_for(chrono::seconds(3));
}

int main() {
	Threadpool tp;
	vector<future<void>> res;

	for (int i = 0; i < 5; ++i) {
		res.emplace_back(tp.commit(test));
	}

	for (auto& f : res) {
		f.get();
	}
	return 0;
}