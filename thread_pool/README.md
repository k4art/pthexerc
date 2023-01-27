# Thread Pool

Implementation of a thread pool.

## Unit Tests

To run unit tests at first download necessary dependencies,

```bash
# assuming your pwd is root of the thread pool project

mkdir lib && cd lib
git clone https://github.com/google/googletest --depth 1
  
```

Then, build using cmake,
```bash
# assuming your pwd is root of the thread pool project

mkdir build && cd build
cmake ..
make all
```

Now you are able to run unit tests using `ctest`.

