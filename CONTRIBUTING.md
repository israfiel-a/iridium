## Contribution Guidelines
Iridium is open to the community, but to maintain a proper cohesive project, certain guidelines must be fulfilled. PRs must meet each of the [major guidelines](#major-guidelines) to be merged into the development branch, and the development branch must meet all of the [minor guidelines](#minor-guidelines) to be merged into the public branch.

These guidelines may be expanded or changed at any time.

---

### Major Guidelines

Table of contents:
- [Code Quality](#code-quality)
- [Documentation](#documentation)
- [Testing](#testing)
- [Community Interaction](#community-interaction)

1. #### Code Quality
    Iridium is not a kernel. It is not critical infastructure, it does not coordinate traffic or route planes. However, a level of stability and security is expected within the engine that must be upheld. This means any added code must be as performant as possible and not crash outright unless extraneous circumstances are in place. There are mechanisms within the engine for dealing with fatal errors, and they **must** be used in **any possible error case**.

    Any added code must also add some sort of value. Garbage PRs--AI included--will simply be closed without merge, and any trusted contributor reserves the right to close such PRs.

    Added code must also be formatted according to the engine's `clang-format` file. This requires a single run-through of the `clang-format` tool, installed by default alongside the `llvm` toolchain.

2. #### Documentation
    Documentation is an incredibly important aspect of maintainability. Documentation also has a strict format within Iridium. Deviation from this format must be corrected before merge.

    - Variables are in `snake_case`.
        - Global variables are in `ir_snake_case`
    - Macros are in `UPPER_CASE`
    - Types are in `ir_snake_case_t`.
        - Enum values are in `ir_snake_case`
    - Functions are in `Ir_PascalCase`.
        - Static/private functions are in `PascalCase`
    - File names are in `PascalCase`.

    Files begin with the following signature:

    ```c
    /**
     * @file [file name]
     * @authors [developer name(s)]
     * @brief [description]
     * @since [version]
     *
     * @copyright (c) [year] the Iridium Development Team
     * This file is under the AGPLv3. For more information on what that 
     * entails, see the LICENSE file provided with the engine.
     */
    ```

    Header files are guarded as follows:

    ```c
    #ifndef IRIDIUM_[parent folder]_[file name]_H
    #define IRIDIUM_[parent folder]_[file name]_H

    // ...

    #endif // IRIDIUM_[parent folder]_[file name]_H
    ```

    As for documenting functions;

    ```c
    /**
     * @name [function name (minus "Ir_")]
     * @authors [developer name(s)]
     * @brief [description]
     * @since [version]
     *
     * @param [param name] - [param description]
     * @returns [return value description]
     */
    ```

    Variables and types are far more simple;

    ```c
    /**
     * @name [variable name]
     * @brief [variable description]
     * @since [version]
     */
    ```

3. #### Testing
    **Your code must run**. It may not cause a crash on any supported platform, and it may not drastically increase resource consumption without proper explanation. It must have been run through AddressSanitizer and UBSan at least once each, and come back clean. A maintainer will also assure these things before merge.

4. #### Community Interaction
    We are not children. There is no real "censorship" within Iridium's community, and there does not need to be. This is not a commercial product. However, unneeded hostility will **not** be tolerated. Over-the-top insults are superfluous. Jabs at a person's character are superfluous. Although civil debate is more than welcome, arguing nonsense with maintainers is useless and will end with you being banned from contribution.

    Keep politics out of development. They are not needed here, and are a dividing factor rather than a unifying one.

    These rules are also enforceable within code comments. Should a comment be found that violates community interaction guidelines, it will be removed/replaced.

---

### Minor Guidelines

Table of contents:
- [Comments](#comments)

1. #### Comments
    Comments are an incredibly important part of documentation. However, comments quickly become garbage--comment rot. Take the following code samples:

    ```c
    // includes library.h
    #include <library.h>
    ```

    ```c
    return 0; // returns 0
    ```

    What value do these comments add? Nothing. Such comments should be removed.

2. #### Casting
    Casting can be very powerful or very destructive. Implicit casts are allowed, except for one outlier; function return values. Function return values should always be taken into account, they're there for a reason. Whenever you do not care about the return value of a given function, you **must** cast it to void.

3. #### Function Alternatives
    When you have two functions that carry out the same process with different parameters, for example a function that defiens an object with or without a format string, a title distinction must be made. This is in the form of one or two characters at the function name's end that describe this functionality change. Take, for example, the following functions from within `Logging.h`:

    ```c
    // ...

    [[gnu::always_inline]] [[nodiscard("Expression result unused.")]]
    inline ir_loggable_t Ir_CreateLoggable(const char *title,
                                        const char *description,
                                        const char *context)
    {
        return (ir_loggable_t){ir_log, title, description, context};
    }

    [[gnu::format(printf, 2, 3)]] [[nodiscard("Expression result unused.")]]
    ir_loggable_t Ir_CreateLoggableDF(const char *title,
                                const char *description_format, ...);

    [[gnu::format(printf, 3, 4)]] [[nodiscard("Expression result unused.")]]
    ir_loggable_t Ir_CreateLoggableCF(const char *title, const char *description,
                                const char *context_format, ...);

    // ...
    ```

    These three functions do, functionally, the same thing. They take some strings, create a loggable object, and return it. The difference is that two of them use format strings, each in different places. So they are suffixed with a `DF` (description format) and `CF` (context format).