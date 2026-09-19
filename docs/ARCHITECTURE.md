# mvnex — Architecture

This document describes the current internal architecture of `mvnex`.

The project is organized as a small layered CLI application:

- `domain` contains project and dependency models/rules.
- `application` contains use cases, ports, and application-level errors.
- `infrastructure` contains concrete adapters such as CLI, filesystem, HTTP, dependency resolution, Maven, and project generation.

The goal is to keep terminal concerns, application orchestration, domain rules, dependency resolution, Maven integration, filesystem operations, and project generation separated.

---

# High-Level Architecture

```text
                ┌────────────────────────┐
                │ infrastructure/cli      │
                │                        │
                │ routing, commands,     │
                │ output, prompt, args   │
                └───────────┬────────────┘
                            │
                            ▼
                ┌────────────────────────┐
                │ application            │
                │                        │
                │ use cases + ports      │
                └───────────┬────────────┘
                            │
              ┌─────────────┼─────────────┐
              ▼             ▼             ▼
          domain       infrastructure  infrastructure
          models       project         dependency/http/
                       generation      maven/filesystem
                                       adapters
```

CLI commands adapt terminal input into application requests.

Use cases coordinate domain validation and infrastructure ports.

Infrastructure adapters implement concrete terminal, filesystem, HTTP, dependency resolver, Maven, POM editing, and project generation behavior.

---

# Current Structure

```text
src/
├── main.cpp
│
├── application/
│   ├── add/
│   │   ├── AddUseCase.h
│   │   └── AddUseCase.cpp
│   │
│   ├── errors/
│   │   ├── DependencyNotFound.h
│   │   ├── DependencyResolutionError.h
│   │   ├── DependencyResolverUnavailable.h
│   │   └── MultipleDependencyMatches.h
│   │
│   ├── init/
│   │   ├── InitUseCase.h
│   │   └── InitUseCase.cpp
│   │
│   └── ports/
│       ├── DependencyResolver.h
│       ├── HttpClient.h
│       ├── MavenChecker.h
│       ├── ProgressReporter.h
│       ├── ProjectCreator.h
│       └── ProjectDependencyRepository.h
│
├── domain/
│   ├── dependency/
│   │   ├── DependencyRequest.h
│   │   ├── DependencyRequest.cpp
│   │   ├── ResolvedDependency.h
│   │   └── ResolvedDependency.cpp
│   │
│   └── project/
│       ├── ProjectConfig.h
│       ├── ProjectNaming.h
│       ├── ProjectNaming.cpp
│       ├── ProjectValidator.h
│       └── ProjectValidator.cpp
│
└── infrastructure/
    ├── cli/
    │   ├── arguments/
    │   ├── command/
    │   ├── commands/
    │   │   ├── add/
    │   │   └── init/
    │   ├── dependency/
    │   ├── output/
    │   ├── prompt/
    │   └── routing/
    │
    ├── dependency/
    │   ├── CompositeDependencyResolver.h
    │   ├── CompositeDependencyResolver.cpp
    │   ├── DepsDevDependencyResolver.h
    │   ├── DepsDevDependencyResolver.cpp
    │   ├── MavenCentralDependencyResolver.h
    │   └── MavenCentralDependencyResolver.cpp
    │
    ├── filesystem/
    │   ├── FileSystem.h
    │   └── FileSystem.cpp
    │
    ├── http/
    │   ├── CprHttpClient.h
    │   └── CprHttpClient.cpp
    │
    ├── maven/
    │   ├── LocalMavenChecker.h
    │   ├── LocalMavenChecker.cpp
    │   ├── MavenWrapperFiles.h
    │   ├── MavenWrapperFiles.cpp
    │   ├── MavenWrapperGenerator.h
    │   └── MavenWrapperGenerator.cpp
    │
    └── project/
        ├── PomProjectDependencyRepository.h
        ├── PomProjectDependencyRepository.cpp
        ├── ProjectGenerator.h
        └── ProjectGenerator.cpp
```

`main.cpp` is the composition root. It creates concrete adapters, wires use cases, registers commands, and starts the command router.

---

# Domain Layer

The domain layer contains rules and data structures that are independent of terminal I/O, filesystem details, and Maven process execution.

Current domain modules:

```text
domain/project/
├── ProjectConfig.h
├── ProjectNaming.h
├── ProjectNaming.cpp
├── ProjectValidator.h
└── ProjectValidator.cpp
```

`ProjectConfig` describes the project to create.

`ProjectNaming` derives Java package-safe names from project names.

`ProjectValidator` validates project names, Java package names, group IDs, and supported Java versions.

```text
domain/dependency/
├── DependencyRequest.h
├── DependencyRequest.cpp
├── ResolvedDependency.h
└── ResolvedDependency.cpp
```

`DependencyRequest` represents what the user asked for before resolution:

- search term, such as `lombok`
- search term with version, such as `lombok:1.18.48`
- coordinate, such as `org.projectlombok:lombok`
- coordinate with version, such as `org.projectlombok:lombok:1.18.48`

`ResolvedDependency` represents an exact Maven dependency that can be written to a POM: `groupId`, `artifactId`, and `version`.

---

# Application Layer

The application layer contains use cases and ports.

Current use cases:

```text
application/init/
├── InitUseCase.h
└── InitUseCase.cpp

application/add/
├── AddUseCase.h
└── AddUseCase.cpp
```

`InitUseCase` validates the requested project, creates it through the `ProjectCreator` port, checks local Maven through the `MavenChecker` port, and returns an `InitUseCaseResult`.

`AddUseCase` resolves requested dependencies through the `DependencyResolver` port, checks existing project dependencies through the `ProjectDependencyRepository` port, adds missing dependencies, and returns which dependencies were added or skipped.

Current ports:

```text
application/ports/
├── DependencyResolver.h
├── HttpClient.h
├── MavenChecker.h
├── ProgressReporter.h
├── ProjectCreator.h
└── ProjectDependencyRepository.h
```

Ports keep application code independent from concrete filesystem, Maven, HTTP, dependency provider, POM editing, project generation, and terminal implementations.

Application errors under `application/errors` describe dependency-resolution failures in use-case terms:

- dependency not found
- dependency resolver unavailable
- multiple dependency matches
- generic dependency resolution failure

---

# CLI Infrastructure

The CLI infrastructure layer handles terminal input and output.

Current CLI modules:

```text
infrastructure/cli/
├── arguments/
├── command/
├── commands/
├── dependency/
├── output/
├── prompt/
└── routing/
```

## Command Model

`infrastructure/cli/command` defines the common command contract and metadata:

- `Command`
- `CommandMetadata`
- `CommandOption`

Command metadata drives global help, command-specific help, and parser option registration.

## Routing

`CommandRegistry` stores available commands.

`CommandRouter` handles global CLI behavior:

- no arguments
- `--help` / `-h`
- `--version` / `-v`
- command lookup
- unknown command errors

The router does not know command-specific options.

## Commands

Concrete CLI commands live under:

```text
infrastructure/cli/commands/
├── init/
└── add/
```

Commands should adapt terminal input into application calls.

They should not write project files, run Maven directly, or contain low-level domain/infrastructure logic.

`InitCommand` currently:

- registers its metadata and options
- parses CLI arguments
- renders command help
- collects missing project config through `InitConfigCollector`
- asks interactive wrapper questions when needed
- calls `InitUseCase`
- delegates display to output adapters

`AddCommand` currently:

- registers dependency-related command metadata and options
- parses CLI arguments
- translates dependency expressions with `DependencyArgumentsParser`
- calls `AddUseCase`
- handles ambiguous dependency matches with an interactive selector
- allows searching again when candidates are not useful
- delegates display to output adapters

## Output

Output concerns live under:

```text
infrastructure/cli/output/
├── ConsoleOutput.h
├── ConsoleOutput.cpp
├── ConsoleProgressReporter.h
├── ConsoleProgressReporter.cpp
├── HelpPrinter.h
├── HelpPrinter.cpp
└── Style.h
```

`ConsoleOutput` centralizes terminal messages.

`HelpPrinter` renders global and command-specific help from command metadata.

`ConsoleProgressReporter` implements the `ProgressReporter` application port and displays spinner output for long-running operations such as Maven Wrapper generation.

---

# Project Initialization Flow

```text
InitCommand
    │
    ▼
InitConfigCollector
    │
    ▼
InitUseCase
    │
    ├── ProjectValidator
    ├── ProjectCreator port
    └── MavenChecker port
```

Concrete adapters:

```text
infrastructure/project/ProjectGenerator
infrastructure/filesystem/FileSystem
infrastructure/maven/MavenWrapperGenerator
infrastructure/maven/LocalMavenChecker
```

`ProjectGenerator` coordinates project file generation.

`FileSystem` owns concrete filesystem operations such as creating directories, writing files, checking existence, and setting executable permissions.

`MavenWrapperGenerator` owns Maven Wrapper generation and delegates file writes to `FileSystem`.

`LocalMavenChecker` checks whether local Maven is available.

---

# Dependency Add Flow

```text
AddCommand
    │
    ▼
DependencyArgumentsParser
    │
    ▼
AddUseCase
    │
    ├── DependencyResolver port
    │   └── CompositeDependencyResolver
    │       ├── Sonatype Central search
    │       ├── Maven Central search
    │       └── deps.dev coordinate fallback
    │
    └── ProjectDependencyRepository port
        └── PomProjectDependencyRepository
```

`DependencyArgumentsParser` is CLI infrastructure. It translates terminal input into domain requests before the application layer runs.

Examples:

```text
lombok                                  -> SearchTerm
lombok:1.18.48                         -> SearchTermWithVersion
org.projectlombok:lombok               -> Coordinate
org.projectlombok:lombok:1.18.48       -> CoordinateWithVersion
```

`CompositeDependencyResolver` asks multiple dependency providers and uses the best available result. Maven Central providers handle search terms and coordinates. The deps.dev adapter is a coordinate fallback.

If dependency resolution returns several plausible matches, the application throws `MultipleDependencyMatches`. The CLI catches that error, shows a selector, and retries the use case with the selected exact coordinate.

`PomProjectDependencyRepository` searches upward from the current directory for the nearest `pom.xml`. It loads existing dependencies into an in-memory set for fast duplicate checks, skips dependencies that already exist or were duplicated in the same command, and writes the remaining dependencies into the POM.

Current POM editing is intentionally simple string-based insertion. Structural XML parsing and deeper Maven model awareness remain future work.

---

# Dependency Direction

Preferred dependency direction:

```text
infrastructure -> application -> domain
```

The application layer may depend on domain types and application ports.

Infrastructure adapters may implement application ports.

Domain code should not depend on application or infrastructure.

CLI code should not bypass use cases for application behavior.

---

# Transactional Project Generation

Project generation should avoid leaving partially-created projects behind.

Potential flow:

```text
validate
   │
   ▼
create project directory
   │
   ▼
generate files
   │
   ├── success ──► keep project
   │
   └── failure ──► remove files created by mvnex
```

Rollback must only delete a directory known to have been created by the current `mvnex` operation.

Existing user directories must never be deleted as part of rollback.

---

# Future Dependency Work

The dependency architecture is in place, but several areas should become more robust:

- replace string-based POM editing with structural XML parsing
- support `dependencyManagement`
- understand parent POMs and multi-module projects
- improve dependency ranking and provider scoring
- add `remove`, `update`, and `outdated` on top of the same dependency model and project repository port
