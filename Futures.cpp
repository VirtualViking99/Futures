// Futures.cpp : This file contains the 'main' function. Program execution begins and ends there.
// 
// Futures: futures provide a mechanism to access the results of an asynchronous operation
// 
// a Synchronous operation blocks a process until the operation completes, an asynchronous operation  is 
// non-blocking and only initiates the operation.  the caller could discover completion by some
// other mechanism later.  in this case, we will use futures
// 
// 
//          Op.1             Op. 1
//          |                |
//          |                |
//          |                |
//          |         Op. 2  |         Op. 2
//          |________        |_________
//                   |       |         |
//                   |       |         |
//                   |       |         |
//          _________|       |_________|
//          |                |
//          |                |
//          |                |
//          |                |
//          |                |
// 
// Left side represents synchronous tasks, after operational initial operation2 1 has to hold untill 2 finishes, only then can 1 proceed to execution
// 
// Right side represents asynchronous operation, after op1 init's op2, still op1 can progress while 2 is on execution.  when
// result of 2 is ready, it can transfer that result via futures to op 1, 1 can proceed UNTIL it needs the result from 2,
// then ask for the result form op. 2.
// 
//  
// STEPS:
//      1. the creator of the asynchronous task has to obtain the future object associated with asynchronous task
// 
//      2. when creator needs the result of that async task, it calls a get() method on the furure
// 
//      3. the get() method may block if the asynchronous operation has not yet completed its execution
// 
//      4. When the asynchronous operation is ready to send a result to the creator, it can do so by modifying teh shared state that is linked to the creators std::future object
//

#include <iostream>
#include <future>       // this allows us to use the futures object and the asynchronous task
#include<string>
//------------------------------
//Parallel accumulate
#include<numeric>

int find_answer_how_old_universe_is()
{
    return 5000;
}
 
void do_other_calculations()
{
    std::cout << "doing other stuff \n";
}

void run_code()
{
    // 1. create the task using find how old function.
    // normally we run functions which take a long time to complete in asynchronous task
    // so we can consider this task as a background task as well
    // 

                    // We have acquired the future associated with this particular task in the construction.
    std::future<int> the_answer_future = std::async(find_answer_how_old_universe_is);

    // 2. since we have the future in this thread, we can do other stuff in the main thread
    do_other_calculations();

    // 3. when we need the result of the asynchronous  task we can get the results, if the asynchronous task is finished
    // but if the asynchronous task has not completed, the main thread will be blocked untill that task is finished
    std::cout << "The answer is : " << the_answer_future.get() << std::endl;
}





/*------------------------------------------------------------------------------------------------------*/

/*
Asynchronous tasks:



std::async
    asynch(std::launch policy, Function&& f, Args&&... args);


    in it's constructor it will allow you specify the launch policy and the function to run 
on the asynch tasks, and argument for that particular function.

for launch policy you can specify std::launch::async or std::launch::deferred.  or you can use both 
of them with a pipe operator

    1. std::launch::async - this launch policy will tell compoiler to run function on a sepparate thread

    2. std::launch::deferred - this launch policy will make the asynchonous task run in the creator thread, when the future.gete() is called

    3. using both tells the compiler to decide which way to run the asynchronous task


*/
 
//* Each one of these functions is going to run in the sepparate asynchronous task*/





void printing()
{
    std::cout << "printing() runs on- " << std::this_thread::get_id() << std::endl;
}

int addition(int x, int y)
{
    std::cout << "adition() runs on- " << std::this_thread::get_id() << std::endl;
    return x + y;
}


int subtraction(int x, int y)
{
    std::cout << "subtraction() runs on- " << std::this_thread::get_id() << std::endl;
    return x - y;
}

void run_code2()
{
    std::cout << "runcode2() thread id- " << std::this_thread::get_id() << std::endl;
    int x{ 100 };
    int y{ 50 };


    //3 async tasks
    // we have to acquire the associated feature when we initiate the task
    // 
    //
    // in construction statement we acquired the future associated with the asynch task
    // 
    // we have to provide approptiate template parameters for the future
    // for these, we have to specify the return type of the function that is going to 
    // execute in async task 
    //
    std::future<void> f1 = std::async(std::launch::async, printing);

    std::future<int> f2 = std::async(std::launch::deferred, addition, x, y);
    
    std::future<int> f3 = std::async(std::launch::deferred | std::launch::async, subtraction, x, y);

    f1.get();

    std::cout << "The value recieved using f2 future -" << f2.get() << std::endl;
    
    std::cout << "The value recieved using f3 future -" << f3.get() << std::endl;


}



/*
Expected output for c++14 standard

Main Thread ID: -21364
runcode2() thread id- 21364
printing() runs on- 27772
subtraction() runs on- 17120
adition() runs on- 21364
The value recieved using f2 future -150
The value recieved using f3 future -50


*/

/*
Recieved outout for c++20 standard

Main Thread ID: -22328
runcode2() thread id- 22328
printing() runs on- 29152
subtraction() runs on- 22584
The value recieved using f2 future -adition() runs on- 22328
150
The value recieved using f3 future -50


*/


/*------------------------------------------------------------------------------------------------------*/

/*
    Implementing the paralel version of accumulate

    Parallel accumulate with async task

    We've already implementd accumulate algorithm paralel when we discused threads
There we found out the numebr of threads to run accumulate paralel to avoid over=\
subscription and heavy overhead

    then we divided our input list to blocks using our thread input counts, and then
we called accumulate for each of the blocks in a sepparate thread, finally we called acucmulate on 
reduced results list

    BUT we don't need to have the number of thread count beforehand, because we are going to use
the asynchronous function with recursive function calls

-------------------------------------------------------------------------------------------------
                                    Long Data Structure
-------------------------------------------------------------------------------------------------
                             ____________________________________
                            |        Sum of A + accu(B)          |
                            |____________________________________|
----------------------------------------             --------------------------------------------
|                   A                   |            |                      B                   |
----------------------------------------             --------------------------------------------

            _____________________
           | Sum of C+ accu() D  |
           |_____________________|

 ----------------     ----------------- 
|       C       |    |        D        |   
 ---------------      -----------------
       _____________
      |   Sum of C  |
      |_____________|
 ---------------
|       C      |
 --------------


    As we first check wether input list contains at least minimum elelment count to avoid overhead
if we don't have, we have to accumulate in current thread and return. but if input list hasthe
wlwment count morethan minimum, we will divide to two parts and call parralel accumulate recursively
over two parts

 



*/

//Minimum element count, so cost of respawning and managing thread will not be issue to avoid overhead.
int MIN_ELEMENT_COUNT = 1000;

template<typename iterator>
int parallel_accumulate(iterator begin, iterator end)
{
    //Check wether the length of the input is greater than the minimum element count
    long length = std::distance(begin, end);    //https://en.cppreference.com/w/cpp/iterator/distance

    //at least runs 1000 element
    if (length <= MIN_ELEMENT_COUNT)
    {
        //accumuate in current thread and return the resuly
        std::cout << std::this_thread::get_id() << std::endl;
        return std::accumulate(begin, end, 0);
    }
    //but if greater..
    //divide the input into two parts and call below accumlate function recursively for each part
    iterator mid = begin;
    std::advance(mid, (length + 1) / 2);

    //for first part recursive call made in current thread


    //recursive to all parallel_accumulate in this thread first


    //second made recursive call using std::async task, and used both launch parameters, so
    // compiler will decide wether async runs in sepparate or current thread, IOT avoid oversubscription
    // bc compiler decides based on existing CPU resources

    std::future<int> f1 = std::async(std::launch::deferred | std::launch::async, 
        parallel_accumulate<iterator>, mid, end);
    

    //finally, addded the result of each of those parts together and returned it
    auto sum = parallel_accumulate(begin, mid);

    //acquire the future, when creating the async task
    return sum + f1.get();


    //specified full type for future but you can sue auto keyword
}

void runcode3()
{
    std::vector<int> v(10000, 1);
    std::cout << "The sum is " << parallel_accumulate(v.begin(), v.end()) << '\n';
}


int main()
{
    //run_code();
    //std::cout<<"Main Thread ID: -" << std::this_thread::get_id() << std::endl;
    //run_code2();

    runcode3();
}

/*
expected output---


67621488
27064

10284
30444
21488
21192
229219312
24300
22392
1152
12688

14440
26692
19184
The sum is 10000



   // You get thread ID's used by algorithm, some used multi times because
for some async task compiler has to use the same thread to run that particular task,
finally, you can see accumulated result as 10,000


*/