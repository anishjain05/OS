
The program initializes semaphores and variables to control access to the bridge, the number of cars on the bridge (on_bridge), and the total number of cars crossed (total_crossed).

Two functions, left and right, represent cars coming from the left and right sides, respectively.
The crossing function simulates the time it takes for a car to cross the bridge, and it prints a message indicating the direction and car number.

The main function initializes semaphores, gets user input for the number of cars on the left and right, and creates threads for each car.
The program waits for all threads to finish using pthread_join.

Semaphores (bridge and mutex) are used to control access to the bridge and prevent race conditions.
The on_bridge variable tracks the number of cars currently on the bridge, ensuring it does not exceed the maximum capacity (MAX_CARS).
The total_crossed variable is updated when a car successfully crosses the bridge, and the program terminates when all cars have crossed.

The use of semaphores ensures mutually exclusive access to the bridge and avoids race conditions. The mutex semaphore protects critical sections where shared variables are accessed.
The program uses a condition variable (all_crossed) to efficiently wait for all cars to cross, preventing busy-waiting and unnecessary resource consumption.
Proper unlocking and signaling mechanisms are employed to avoid deadlocks and ensure the correct flow of execution.