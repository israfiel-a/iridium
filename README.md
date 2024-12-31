## Iridium
Iridium is a 3D game engine built using [C23](https://en.wikipedia.org/wiki/C23_(C_standard_revision)). It is built expressly for performance and stability. Iridium is [open to the community](./LICENSE.md), any/all improvements are welcomed.

---

### Dependencies
Iridium is built to be as light as possible on the dependency side of things. However, we can't reinvent **every** wheel, so the project still requires:

- [libVulkan](https://www.vulkan.org/) (all platforms)
- [libWayland](https://wayland.freedesktop.org/) (Wayland Linux)

---

### Compatibility
While we don't aim to be able to run on traffic controllers, compatibility is still an important aspect of Iridium. However, our time is also not unlimited, so until someone comes along with a use case, not every niche system will be supported. For now, Iridium is restricted to:

- [Microsoft Windows](https://www.microsoft.com/en-us/windows/): Consistent testing on  Windows 10 and onward. However, 7/8 should be possible with a bit of tinkering.
- [Linux](https://kernel.org/): Fairly close to any newer-ish version of Linux, so long as you've got a desktop environment installed running Wayland.

[MacOS](https://support.apple.com/mac) is planned, but firmly on the backburner. If there's an entry to this list that you believe would be simple (or at least doable) to implement, open an Issue.

---

### Contributions
Contributions to the engine must follow our [contribution guidelines](./CONTRIBUTING.md). PRs will **not** be merged unless they meet said standards. To summarize:

- [Document your stuff.](./CONTRIBUTING.md#documentation)
- [Properly test your stuff.](./CONTRIBUTING.md#testing)
- [Don't be a dick.](./CONTRIBUTING.md#community-interaction)

Beyond those golden rules, help out where you can, add new functionality where it's needed, and generally have fun. Well, as much fun as you can have in C.