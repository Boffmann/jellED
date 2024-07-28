# Run and Debug Tests

To run and debug tests, first compile the package with debug information:

```
$ cd <path to build dir>
$ cmake -DCMAKE_BUILD_TYPE=Debug ..
$ make
```

Afterwards, execute the tests using `ctest`.
To run a specific test, use the `-R` option:

```
$ ctest -R <test_name>
```

Example:

```
$ ctest -R BandpassFilterTest.TestApplyBandpass
```
