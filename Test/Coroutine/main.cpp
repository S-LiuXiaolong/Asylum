#include <iostream>
#include <thread>
#include <mutex>
#include <functional>

class InterruptedException : public std::exception {
};

class AsyncThread {
public:
	AsyncThread() {
		std::unique_lock<std::mutex> lock(mutex);
		thread.reset(new std::thread(std::bind(&AsyncThread::run, this)));
		conditionVar.wait(lock); // wait for the thread to start
	}
	~AsyncThread() {
		{
			std::lock_guard<std::mutex> _(mutex);
			quit = true;
		}
		conditionVar.notify_all();
		thread->join();
	}
	void run() {
		try {
			yield();
			for (int i = 0; i < 7; ++i) {
				std::cout << i << std::endl;
				yield();
			}
		} catch (InterruptedException& e) {
			return;
		}
		std::lock_guard<std::mutex> lock(mutex);
		quit = true;
		conditionVar.notify_all();
	}
	void yield() {
		std::unique_lock<std::mutex> lock(mutex);
		conditionVar.notify_all();
		conditionVar.wait(lock);
		if (quit) {
			throw InterruptedException();
		}
	}
	void step() {
		std::unique_lock<std::mutex> lock(mutex);
		if (!quit) {
			conditionVar.notify_all();
			conditionVar.wait(lock);
		}
	}
private:
	std::unique_ptr<std::thread> thread;
	std::condition_variable conditionVar;
	std::mutex mutex;
	bool quit = false;
};

int main() {
	AsyncThread asyncThread;
	for (int i = 0; i < 3; ++i) {
		std::cout << "main: " << i << std::endl;
		asyncThread.step();
	}
}