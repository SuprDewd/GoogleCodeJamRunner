# Google Code Jam Runner
This is a wrapper library for C++ solutions to Google Code Jam problems. This
wrapper

- takes care of test cases
- let's you run your solution on a specific set of test cases (with the **-o** flag)
- tells you on which test case your solution crashed (planned feature)
- gives you a nice overview of the progress your solution is making through the test cases
- allows you to spawn multiple processes, even on multiple machines, and distribute the test caeses among them (with the **-d** flag and a simple config file)

## Usage
Just copy **gcj.h**, **include.h**, and **template.cpp** to a directory where
your solutions are located.

To start coding a new solution, create a new file
with the same contents as **template.cpp**. Inside the **input()** function you
should read a single test case from the provided **cin** stream, and store
required data in member variables. Inside the **solve()** function you should
solve a single test case described by the member variables you populated in the
**input()** function, and output the solution to the provedid **cout** stream.

To run the program, start by compiling your solution like usually (**g++ -Wall
-O2 solution.cpp -o run**). Then run the program, and pass a command line
argument specifying a test file to run.

