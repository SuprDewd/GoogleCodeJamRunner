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

To start coding a new solution, create a new file with the same contents as
**template.cpp**, and follow the **TODO**'s.

To run the program, start by compiling your solution like usually (**g++ -Wall
-O2 solution.cpp -o run**). Then run the program, and pass a command line
argument specifying a test file to run. For example, **./run test-file >
test-file.out**.

### Distributing test cases
Create a file **dist.conf** with one or more lines. Each line contains a
hostname (or any kind of hostname string that the **ssh** and **scp** commands
accept), followed by the number of jobs that should be run on that machine.
Note that you should be able to **ssh** into the given hostnames without having
to specify a password. Then you can run an input file in distributed mode by
calling **./run test-file -d dist.conf > test-file.out**.

