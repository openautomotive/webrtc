# WebRTC coding style guide

## C++

WebRTC follows the [Chromium][chr-style] and [Google][goog-style] C++
style guides, unless an exception is listed below. In cases where they
conflict, the Chromium style guide trumps the Google style guide, and
the exceptions in this file trump them both.

[chr-style]: https://chromium.googlesource.com/chromium/src/+/master/styleguide/c++/c++.md
[goog-style]: https://google.github.io/styleguide/cppguide.html

Some older parts of the code violate the style guide in various ways.

* If making small changes to such code, follow the style guide when
  it’s reasonable to do so, but in matters of formatting etc., it is
  often better to be consistent with the surrounding code.
* If making large changes to such code, consider first cleaning it up
  in a separate CL.

### ArrayView

When passing an array of values to a function, use `rtc::ArrayView`
whenever possible—that is, whenever the callee may not modify the
array itself (the number of elements, etc.). This is always the case
for read-only arguments, but is also true when the callee may modify
the elements but not add or remove them.

For function arguments:

instead of                          | use
------------------------------------|-------------------
`const std::vector<T>&`             | `ArrayView<const T>`
`const T* ptr, size_t num_elements` | `ArrayView<const T>`
`T* ptr, size_t num_elements`       | `ArrayView<T>`

(Why? Because it’s *safer* than pointer+size, largely because it’s
trivial to figure out what the element count is when reading the code;
and because it’s *more flexible* than e.g. `const std::vector<T>&`,
since the caller isn’t forced to be storing the elements in a
`std::vector`.)

Since `ArrayView` can’t be used when you want to transfer ownership of
the array, it is generally not useful as a return value, except in
cases where the return value is just a “pointer” into an array that’s
owned elsewhere.

## C

There’s a substantial chunk of legacy C code in WebRTC, and a lot of
it is old enough that it violates the parts of the C++ style guide
that also applies to C (naming etc.) for the simple reason that it
pre-dates the use of the current C++ style guide for this code base.

* If making small changes to C code, mimic the style of the
  surrounding code.
* If making large changes to C code, consider converting the whole
  thing to C++ first.

## Build files

### Conditional compilation with the C preprocessor

Avoid using the C preprocessor to conditionally enable or disable
pieces of code. But if you can’t avoid it, introduce a gn variable,
and then set a preprocessor constant to either 0 or 1 in the build
targets that need it:

```
if (apm_debug_dump) {
  defines = [ "WEBRTC_APM_DEBUG_DUMP=1" ]
} else {
  defines = [ "WEBRTC_APM_DEBUG_DUMP=0" ]
}
```

In the C, C++, or Objective-C files, use `#if` when testing the flag,
not `#ifdef` or `#if defined()`:

```
#if WEBRTC_APM_DEBUG_DUMP
// One way.
#else
// Or another.
#endif
```

When combined with the `-Wundef` compiler option, this produces
compile time warnings if preprocessor symbols are misspelled, or used
without corresponding build rules to set them.
