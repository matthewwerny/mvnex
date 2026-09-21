# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this is

`mvnex` is a native C++20 CLI that adds an npm-like developer experience on top of Maven (`mvnex init`, `mvnex add <dep>`). Core rule: **mvnex enhances Maven, it doesn't replace it** — `pom.xml` stays the source of truth, generated/modified projects must remain plain Maven projects, and where Maven already does something, mvnex should orchestrate Maven rather than reimplement it. See `docs/VISION.md` and `docs/ARCHITECTURE.md`.

## Build & run

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release   # first configure fetches replxx + cpr via FetchContent (needs network)
cmake --build build --config Release
./build/mvnex --version                          # CI smoke test; Windows binary is build/Release/mvnex.exe
./build/mvnex init my-app --group-id com.example --java 21 --no-wrapper
./build/mvnex add lombok junit-jupiter --scope test   # run inside (or below) a directory with pom.xml
```

- Requires CMake ≥ 3.21 (per `CMakeLists.txt`) and a C++20 compiler. `compile_commands.json` is exported to `build/`.
- **Every `.cpp` file must be added manually to the `add_executable(mvnex ...)` list in `CMakeLists.txt`** — there is no glob. Headers are included relative to `src/` (e.g. `#include "application/ports/HttpClient.h"`).
- Version comes from `project(mvnex VERSION ...)` in CMake and reaches code as the `MVNEX_VERSION` define.
- Platform quirks in CMake: MSVC builds shared libs with the DLL runtime (DLLs are copied next to the exe post-build); macOS uses the system curl for cpr.
- **No tests or linter currently exist.** `include(CTest)` is present but no tests are registered (an earlier `tests/` suite was removed in commit `9589b32`). Verification means building and exercising the binary manually.

CI (`.github/workflows/build.yml`) builds on macOS/Linux/Windows on every push/PR; pushing a `v*` tag packages zips and creates a GitHub prerelease.

## Architecture

Layered / ports-and-adapters, dependency direction `infrastructure -> application -> domain`:

- `src/domain/` — pure models and rules (`ProjectConfig`, `ProjectValidator`, `ProjectNaming`, `DependencyRequest`, `ResolvedDependency`). No I/O.
- `src/application/` — use cases (`InitUseCase`, `AddUseCase`), port interfaces in `ports/`, and use-case-level exceptions in `errors/`.
- `src/infrastructure/` — adapters implementing ports (filesystem, cpr HTTP, Maven process calls, POM editing, project generation, dependency resolvers) plus the entire CLI layer under `cli/`.
- `src/main.cpp` is the **composition root**: all concrete objects are stack-constructed and wired by reference there; commands are registered into `CommandRegistry` and `CommandRouter` dispatches.

### CLI layer conventions

- A command implements `Command` (`metadata()` + `execute(argc, argv)`). Its `CommandMetadata` drives both help rendering (`HelpPrinter`) and option registration for `ArgumentParser`. `CommandRouter` only handles global concerns (`--help`, `--version`, unknown command) and knows nothing about command options.
- Commands only adapt terminal input → use-case calls and delegate display to `ConsoleOutput`. They must not write files, run Maven, or hold domain logic.
- Adding a command = new class under `infrastructure/cli/commands/<name>/`, a use case under `application/<name>/`, any new ports/adapters, wiring in `main.cpp`, and the `.cpp` files in `CMakeLists.txt`.
- Interactive input uses `Prompt` (backed by replxx); long operations report through the `ProgressReporter` port (spinner in `ConsoleProgressReporter`).

### `add` flow — non-obvious behaviors

- `DependencyArgumentsParser` (CLI layer) turns `lombok`, `lombok:1.2`, `g:a`, `g:a:v` into a `DependencyRequest`. Scope comes from CLI flags, never from resolvers; `AddUseCase` attaches it after resolution.
- `CompositeDependencyResolver` runs all resolvers **concurrently** (`std::async`) — Sonatype Central search, Maven Central search (both `MavenCentralDependencyResolver` with different URLs/timeouts), and deps.dev (coordinate fallback). The **first** resolver to return a result or a `MultipleDependencyMatches` wins; failures are aggregated only if all fail. Resolver implementations are therefore called from multiple threads and must be safe for concurrent `const` calls.
- Ambiguity is signaled by throwing `MultipleDependencyMatches`; `AddCommand` catches it, shows a selector (with a "Search again..." option), and re-runs the use case with the chosen exact coordinate.
- `PomProjectDependencyRepository` walks upward to the nearest `pom.xml`, skips existing/duplicate deps, and edits the POM via **simple string insertion** (no XML parser; no `dependencyManagement`, parent POM, or multi-module awareness yet).
- `LocalMavenProjectValidator` runs `./mvnw validate`, else `mvn validate`; missing Maven is a warning, not a failure.

### `init` flow

`InitCommand` → `InitConfigCollector` (prompts for missing values) → `InitUseCase` → `ProjectValidator`, `ProjectCreator` port (`ProjectGenerator` → `FileSystem`, `MavenWrapperGenerator`), `MavenChecker` port. It refuses to overwrite an existing directory; wrapper generation has a fallback when global `mvn` is unavailable. Planned rollback must only ever delete directories created by the current operation.

## Workflow notes

- Commits use Conventional Commits with optional scope: `feat(add): ...`, `fix(init): ...`, `refactor(cli): ...`, `ci: ...`, `build: ...`, `docs: ...`.
- The repo uses OpenSpec (`openspec/`, `/opsx:*` commands and `openspec-*` skills in `.claude/`) for spec-driven changes: propose → apply → archive.
- Keep `README.md` roadmap and `docs/ARCHITECTURE.md` in sync when adding commands or changing layers.
