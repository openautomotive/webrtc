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

### Conditional compilation

First, think long and hard about whether you really need to add
another flag that makes the compiler do different things depending on
whether the flag is set or not. But if you really, really do, the way
to do it is to introduce a gn variable, and then set a preprocessor
constant to either 0 or 1 in the build targets that need it:

```
if (apm_debug_dump) {
  defines = [ "WEBRTC_APM_DEBUG_DUMP=1" ]
} else {
  defines = [ "WEBRTC_APM_DEBUG_DUMP=0" ]
}
```

In the C++ files, use `#if` when testing the flag, not `#ifdef` or
`#if defined()`:

```
#if WEBRTC_APM_DEBUG_DUMP
// One way.
#else
// Or another.
#endif
```

If you do it this way, the compiler will complain if you use
`WEBRTC_APM_DEBUG_DUMP` in translation units where it isn’t being set
by the build system. (If you use the old and bad way—setting the
preprocessor constant to an arbitrary value to enable the feature and
leaving it unset to disable the feature, and then using `#ifdef` to
tell the two cases apart—the compiler won’t be able to tell you if you
use it in translation units where it isn’t being set by the build
system.)

## C

There’s a substantial chunk of legacy C code in WebRTC, and a lot of
it is old enough that it violates the parts of the C++ style guide
that also applies to C (naming etc.) for the simple reason that it
pre-dates the use of the current C++ style guide for this code base.

* If making small changes to C code, mimic the style of the
  surrounding code.
* If making large changes to C code, consider converting the whole
  thing to C++ first.
