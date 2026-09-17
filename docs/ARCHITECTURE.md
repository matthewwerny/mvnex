# mvnex — Architecture

This document describes the current internal architecture of `mvnex`.

The project is organized as a small layered CLI application:

- `domain` contains project rules and domain models.
- `application` contains use cases and ports.
- `infrastructure` contains concrete adapters such as CLI, filesystem, Maven, and project generation.

The goal is to keep terminal concerns, application orchestration, project rules, Maven integration, filesystem operations, and project generation separated.

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
          project      project         maven/filesystem
          rules        generation      adapters
```

CLI commands adapt terminal input into application requests.

Use cases coordinate domain validation and infrastructure ports.

Infrastructure adapters implement concrete terminal, filesystem, Maven, and project generation behavior.

---

# Current Structure

```text
src/
├── main.cpp
│
├── application/
│   ├── init/
│   │   ├── InitUseCase.h
│   │   └── InitUseCase.cpp
│   │
│   └── ports/
│       ├── MavenChecker.h
│       ├── ProgressReporter.h
│       └── ProjectCreator.h
│
├── domain/
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
    │   ├── output/
    │   ├── prompt/
    │   └── routing/
    │
    ├── filesystem/
    │   ├── FileSystem.h
    │   └── FileSystem.cpp
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

---

# Application Layer

The application layer contains use cases and ports.

Current use cases:

```text
application/init/
├── InitUseCase.h
└── InitUseCase.cpp
```

`InitUseCase` validates the requested project, creates it through the `ProjectCreator` port, checks local Maven through the `MavenChecker` port, and returns an `InitUseCaseResult`.

Current ports:

```text
application/ports/
├── MavenChecker.h
├── ProgressReporter.h
└── ProjectCreator.h
```

Ports keep application code independent from concrete filesystem, Maven, project generation, and terminal implementations.

---

# CLI Infrastructure

The CLI infrastructure layer handles terminal input and output.

Current CLI modules:

```text
infrastructure/cli/
├── arguments/
├── command/
├── commands/
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

# Future Dependency Domain

Dependency input should be represented independently from Maven Central or XML.

Possible future structure:

```text
domain/dependency/
├── DependencyRequest.h
├── DependencyParser.h
└── ResolvedDependency.h

application/add/
├── AddDependencyUseCase.h
└── AddDependencyUseCase.cpp

application/ports/
├── DependencyResolver.h
└── PomEditor.h

infrastructure/maven/
├── MavenCentralClient.*
└── PomXmlEditor.*
```

The goal is the same as `init`: keep CLI parsing, application orchestration, domain rules, and infrastructure details separate.
