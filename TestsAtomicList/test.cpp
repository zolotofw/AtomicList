#include "pch.h"
#include "../AtomicList/AtomicList.hpp"
#include <barrier>
#include <vector>
#include <thread>

TEST(TestAtomicList, TestPushFrontAndBackIfEmpty)
{
    int num_threads = 4;

    std::barrier sync_point(num_threads + 1);
    AtomicList<int> test_list;

    std::vector<std::thread> threads;

    int value = 0;
    bool if_push_front_or_back = true;

    for (size_t i = 0; i < num_threads; ++i)
    {
        threads.emplace_back(std::thread([&if_push_front_or_back, &sync_point, &test_list, &value]()
            {
                for (size_t i = 0; i < 3; ++i)
                {
                    if (if_push_front_or_back)
                    {
                        test_list.push_front(++value);
                    }
                    else
                    {
                        test_list.push_back(++value);
                    }
                }
                sync_point.arrive_and_wait();
            }));

        if_push_front_or_back = !if_push_front_or_back;
    }

    for (auto& thr : threads)
    {
        thr.detach();
    }

    sync_point.arrive_and_wait();

    //std::cout << test_list;

    EXPECT_EQ(test_list.size(), 12);
    EXPECT_TRUE(Testing::check_if_any<int>(test_list, { 1,2,3,4,5,6,7,8,9,10,11,12 }));
}

TEST(TestAtomicList, TestPushFrontAndBackIfNotEmpty)
{
    int num_threads = 4;
    std::barrier sync_point(num_threads + 1);
    AtomicList<int> test_list{ 0,0,0,0,0 };

    std::vector<std::thread> threads;

    int value = 0;
    bool switch_push_front_or_back = true;

    for (size_t i = 0; i < num_threads; ++i)
    {
        threads.emplace_back(std::thread([&switch_push_front_or_back, &sync_point, &test_list, &value]()
            {
                for (size_t i = 0; i < 3; ++i)
                {
                    if (switch_push_front_or_back)
                    {
                        test_list.push_front(++value);
                    }
                    else
                    {
                        test_list.push_back(++value);
                    }
                }
                sync_point.arrive_and_wait();
            }));

        switch_push_front_or_back = !switch_push_front_or_back;
    }

    for (auto& thr : threads)
    {
        thr.detach();
    }

    sync_point.arrive_and_wait();

    //std::cout << test_list;

    EXPECT_EQ(test_list.size(), 17);
    EXPECT_TRUE(Testing::check_if_any<int>(test_list, { 1,2,3,4,5,0,0,0,0,0,6,7,8,9,10,11,12 }));
}
