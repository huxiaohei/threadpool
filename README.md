# ThreadPool

* 支持指定最大线程数量
* 支持设置任务过期时间
* 异步等待返回结果

## 案例

```cpp

#include "threadpool.h"
#include "utils.h"
#include <iostream>
#include <string>

void task_0()
{
    std::cout << "task_0 start " << TimeUtils::get_current_time_milliseconds() << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(2));
    std::cout << "task_0 end " << TimeUtils::get_current_time_milliseconds() << std::endl;
}

uint64_t task_1()
{
    uint64_t start_time = TimeUtils::get_current_time_milliseconds();
    std::this_thread::sleep_for(std::chrono::seconds(2));
    return TimeUtils::get_current_time_milliseconds() - start_time;
}

int task_2(int a, int b)
{
    std::this_thread::sleep_for(std::chrono::seconds(2));
    return a + b;
}

int main()
{
    auto threadpool = std::make_shared<ThreadPool>(2);

    threadpool->exec(1000, []() { std::cout << "task will ignore" << std::endl; });
    threadpool->exec(3000, []() { std::cout << "task will exec" << std::endl; });

    std::this_thread::sleep_for(std::chrono::seconds(2));

    threadpool->start();

    threadpool->exec(task_0);

    auto cost_time = threadpool->exec(task_1);
    std::cout << "task_1 cost time: " << cost_time.get() << "ms" << std::endl;

    auto result = threadpool->exec(task_2, 1, 2);
    std::cout << "task_2 result: " << result.get() << std::endl;

    threadpool->exec([]() {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::cout << "lambda task return void" << std::endl;
    });
    int a = 1, b = 2;
    auto lambda_result = threadpool->exec([](int v1, int v2) { return v1 + v2; }, a, b);
    std::cout << "lambda task result: " << lambda_result.get() << std::endl;

    threadpool->wait_for_all_done(std::chrono::milliseconds(5000).count());
    return 0;
}
```
