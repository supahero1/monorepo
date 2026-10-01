Except unless explicitly stated otherwise, if a test is bound to fail and uses some NULL value in it, the failure is expected to be a SIGABRT from an assertion detecting the NULL value and not a SIGSEGV from dereferencing it. In other words, it is tested whether it can be easily debugged if an invalid value is passed to any function.

All tests contain functions named in the format `test_<pass/fail/timeout>__<name>`. Fail means it gets SIGABRT, timeout means it runs too long, and pass means any other situation. A test is considered successful if it passes in the sense that the behavior is as expected, even if it fails or times out.

None of the tests contain a `main` function. Instead, they are compiled with `libtest.c` which dynamically discovers all tests and runs them, each in a separate process. You can pass `--name <name>` to run a specific test (all test names are globally unique), which disables process virtualization, so that its easier to debug with gdb or valgrind.

Even if a test is supposed to fail, proper resource initialization is still necessary in case it in fact does not fail where it was supposed to. Were the resources not initialized properly, the test could have still failed, but in a different, unrelated place.
