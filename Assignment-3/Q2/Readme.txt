Car Thread:

The car thread loads, runs the ride, and unloads until all passengers have completed their rides.
During each cycle, the car signals passengers to board, waits for them to finish boarding, runs the ride, signals passengers to unboard, and waits for them to finish unboarding.
The process repeats until there are no remaining passengers.

Passenger Thread:

Passenger threads attempt to board and unboard the car in a loop until all passengers have completed their rides.
Passengers wait for the car to signal them to board, board the car, signal the car when boarding is complete, wait for the car to signal them to unboard, and unboard the car.
If there are no more rides, passengers exit the loop.

Synchronization:

Semaphores are used for synchronization:
mutex: For mutual exclusion between car and passenger threads.
board_queue: Car signals passengers to board, and passengers signal the car when boarding is complete.
unboard_queue: Car signals passengers to unboard, and passengers signal the car when unboarding is complete.
Semaphores ensure that each step is carried out in the correct order and prevent race conditions.

Avoiding Concurrency Bugs:

The mutex semaphore ensures that only one thread (either car or passenger) can access critical sections at a time.
It prevents multiple threads from modifying shared variables simultaneously, avoiding data corruption.

Semaphores (board_queue and unboard_queue) are used for signaling between the car and passenger threads.
Signaling is done in a way that ensures threads wait for each other at the appropriate synchronization points.
This prevents issues such as passengers attempting to board before the car is ready or the car unloading before passengers have finished boarding.

Loop Conditions:

Passenger threads exit their loop when there are no more rides (remaining_passengers <= 0).
This prevents passenger threads from attempting to board or unboard after the car has completed its rounds, avoiding unnecessary contention.

The loaded_passengers variable is updated carefully to show the correct number of passengers on board.
This prevents inconsistencies that could come if multiple threads tried to update the variable simultaneously.