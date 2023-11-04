# `stdio_usb_simple`: Faster, Simpler Alternative to `stdio_usb`

**Warning:** Unlike the default `stdio_usb`, this module will eventually
deadlock if you print from interrupt handlers (which you shouldn't be doing
anyway). It will give you higher throughput and the ability to take control
and write data directly, though. Pick your poison.

To use this library with `pico-sdk`, you can add it as a submodule:

```bash
git submodule add https://github.com/mordae/pico-stdio-usb-simple.git vendor/pico-stdio-usb-simple
```

Modify your `CMakeLists.txt`:

```cmake
add_subdirectory(vendor/pico-stdio-usb-simple)
target_link_libraries(your_target pico_stdio_usb_simple ...)
```

From you code:

```c
#include <pico/stdio_usb.h>

int main()
{
        stdio_usb_init();
        printf("Hello!\n");
        ...
}
```

You should be able to mix `printf()` with `tud_cdc_write()` like this:

```c
/* Empty the stdio buffers into the USB CDC. */
stdio_flush();

/*
 * Acquire the mutex, blocking other readers, writers and periodic tud_task().
 * You must make sure to call tud_task() frequently enough from now on.
 *
 * This will block all stdio, including UART if you use it.
 */
stdio_usb_lock();

/*
 * Dump a lot of data into the USB stream without newline conversions and
 * other stdio meddling altogether.
 */
while (...) {
        tud_cdc_write(...);
        tud_cdc_write_flush();
        tud_task();
        ...
}

/* Unlock the mutex, allowing the interrupt to run tud_task() for us again. */
stdio_usb_unlock();
```
