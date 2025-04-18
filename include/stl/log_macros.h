#ifndef CONTAINER_LOGF
    #define CONTAINER_LOGF(level, fmt, ...) \
        ::bluestl::logf(level, std::source_location::current(), fmt, ##__VA_ARGS__)
#endif

#ifndef CONTAINER_ASSERT
    #define CONTAINER_ASSERT(expr) \
        ((expr) ? (void)0 : ::bluestl::assert_fail(#expr, __FILE__, __LINE__))
#endif
