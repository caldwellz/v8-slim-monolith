# v8-slim-monolith

This repository builds trimmed-down monolithic static libraries of the V8 JavaScript engine using GitHub Actions, with the following browser-oriented features disabled (to reduce dependencies and binary sizes of downstream targets):

* [WebAssembly](https://v8.dev/docs/wasm-compilation-pipeline)
* [Intl](https://v8.dev/docs/i18n)
* [Sandbox](https://v8.dev/blog/sandbox)
* Most debugging features, including DCHECK and stack traces.

Since the builds are meant to be integrated into my own projects, I manually target specific releases by pushing a new V8_VERSION file, which triggers a build & release of that version.

See embed-example.cpp for the tweaks needed to embed a slim build in your project.
