# mvnex

**A modern developer experience for Maven.**

`mvnex` is an open-source native CLI that brings a simpler, more interactive, npm-like developer experience to Maven projects — without replacing Maven.

```bash
mvnex init my-app
cd my-app

# Planned
mvnex add lombok
mvnex add org.postgresql:postgresql
```

> **mvnex enhances Maven. It doesn't replace it.**

Projects created or modified with `mvnex` remain standard Maven projects.

---

## Why mvnex?

Maven is powerful, mature, and widely used throughout the Java ecosystem.

However, many everyday operations still involve verbose commands, manual `pom.xml` editing, or knowledge of exact Maven coordinates.

For example, adding a dependency normally requires finding its coordinates and manually editing the POM:

```xml
<dependency>
    <groupId>org.projectlombok</groupId>
    <artifactId>lombok</artifactId>
    <version>1.18.48</version>
</dependency>
```

The long-term goal of `mvnex` is to make common workflows simpler:

```bash
mvnex add lombok
```

while still producing a completely standard Maven project.

---

## Design philosophy

### Maven stays Maven

`mvnex` is not a new build system.

It does not replace:

- Maven dependency resolution
- Maven repositories
- Maven lifecycle
- `pom.xml`
- Maven plugins
- existing Maven tooling

Instead, `mvnex` acts as a developer-experience layer around Maven.

The `pom.xml` remains the source of truth.

You should always be able to stop using `mvnex` and continue with:

```bash
mvn compile
mvn test
mvn package
```

without migrating or converting your project.

### No lock-in

A project created with:

```bash
mvnex init my-app
```

should still be a normal Maven project.

A project modified with:

```bash
mvnex add lombok
```

should still contain normal Maven dependency declarations.

`mvnex` should improve the workflow, not create a new ecosystem that users become dependent on.

---

# Current features

## `mvnex init`

Create a new Maven project interactively:

```bash
mvnex init
```

The interactive flow asks for:

- project name
- Maven groupId
- Java version
- whether to generate Maven Wrapper

You can also provide configuration directly:

```bash
mvnex init my-app --group-id com.example --java 21
```

Override the generated Java package when needed:

```bash
mvnex init my-app --package com.example.app
```

By default, `mvnex init` generates Maven Wrapper files so the project can be built with `./mvnw` even when Maven is not installed globally. To skip wrapper generation and rely on a local Maven installation, use:

```bash
mvnex init my-app --no-wrapper
```

Available options:

```text
-g, --group-id <id>       Set Maven groupId
-p, --package <name>      Set Java package name
-j, --java <version>      Set Java version
-w, --no-wrapper          Do not generate Maven Wrapper
-h, --help                Show init help
```

`mvnex init` currently provides:

- interactive project creation
- editable terminal input
- sensible defaults
- project-name validation
- Java package validation
- Java version validation
- Maven-standard directory structure
- automatic package generation from `groupId` and project name
- optional package override with `--package`
- `Main.java` generation
- `pom.xml` generation
- Maven Wrapper generation
- fallback wrapper generation when global `mvn` is unavailable
- optional `--no-wrapper` mode
- protection against existing project directories

Generated structure with Maven Wrapper enabled:

```text
my-app/
├── .mvn/
│   └── wrapper/
│       └── maven-wrapper.properties
├── mvnw
├── mvnw.cmd
├── pom.xml
└── src/
    ├── main/
    │   └── java/
    │       └── com/example/myapp/
    │           └── Main.java
    └── test/
        └── java/
            └── com/example/myapp/
```

The generated project can immediately be built with the wrapper:

```bash
cd my-app
./mvnw package
```

If the project was created with `--no-wrapper`, use local Maven instead:

```bash
mvn package
```

---

# Planned commands

The following commands describe the direction of the project and are **not necessarily implemented yet**.

## Dependencies

```bash
mvnex add lombok
mvnex add lombok:1.18.48
mvnex add org.projectlombok:lombok
mvnex add org.projectlombok:lombok:1.18.48

mvnex remove lombok

mvnex update
mvnex outdated
```

Short dependency names should be resolved through Maven Central when possible.

For ambiguous artifacts, `mvnex` should allow the developer to choose:

```text
◇  Multiple artifacts found
│
│  com.google.inject:guice
│  io.github.replay-framework:guice
│  com.codeborne.replay:guice
│
◇  Select dependency
│  › com.google.inject:guice
```

Explicit Maven coordinates should always be supported.

---

## Install

Planned:

```bash
mvnex install
```

`mvnex install` is intended to prepare a project for development using Maven as the underlying engine.

It should not redefine Maven dependency resolution.

It is also intentionally different from:

```bash
mvn install
```

which executes Maven's lifecycle through the `install` phase and installs the built artifact into the local Maven repository.

---

## Scripts

A future `mvnex.toml` may provide convenient project commands:

```toml
[java]
version = 21

[scripts]
dev = "mvn spring-boot:run"
build = "mvn clean package"
test = "mvn test"
start = "java -jar target/app.jar"
```

Allowing:

```bash
mvnex run dev
mvnex run build
mvnex run test
```

---

## Java management

Potential future commands:

```bash
mvnex java install 21
mvnex java use 21
mvnex java list
```

The goal would be to make the JDK required by a project easier to discover and configure.

---

# Roadmap

## Project creation

- [x] `mvnex init`
- [x] Interactive terminal input
- [x] Project defaults
- [x] Java/package validation
- [x] Maven project generation
- [x] Maven Wrapper generation
- [x] Optional `--no-wrapper` mode
- [x] Optional `--package` override
- [ ] Transactional project generation / rollback
- [ ] Project types
- [ ] Spring Boot template
- [ ] Java library template
- [ ] CLI application template

## Dependencies

- [ ] Dependency expression parser
- [ ] Maven Central search
- [ ] Maven metadata resolution
- [ ] Interactive artifact selection
- [ ] `mvnex add`
- [ ] Explicit dependency versions
- [ ] Multiple dependencies per command
- [ ] Duplicate dependency detection
- [ ] `mvnex remove`
- [ ] `mvnex update`
- [ ] `mvnex outdated`

## Maven integration

- [ ] Structural POM parsing
- [ ] Structural POM modification
- [ ] `dependencyManagement` support
- [ ] Parent POM awareness
- [ ] Multi-module project support

## Workflow

- [ ] `mvnex install`
- [ ] `mvnex.toml`
- [ ] `mvnex run`
- [ ] Project scripts

## Java

- [ ] JDK detection
- [ ] `mvnex java list`
- [ ] `mvnex java use`
- [ ] `mvnex java install`

## Distribution

- [ ] macOS ARM64
- [ ] macOS x86_64
- [ ] Linux x86_64
- [ ] Linux ARM64
- [ ] Windows x86_64
- [ ] GitHub Releases
- [ ] Homebrew
- [ ] WinGet
- [ ] Linux installer

---

# Building from source

### Requirements

- C++20-compatible compiler
- CMake 3.20+
- Git

Clone the repository and build:

```bash
git clone <repository-url>
cd mvnx

cmake -S . -B build
cmake --build build
```

Run:

```bash
./build/mvnex
```

Dependencies required by `mvnex` are managed through CMake where possible.

---

# Technology

`mvnex` is primarily written in **C++20**.

Current and planned technologies include:

- C++20
- CMake
- replxx
- Maven Central APIs
- Maven repository metadata
- XML parsing
- native filesystem APIs

C++ allows `mvnex` to be distributed as a native executable without requiring a JVM merely to run the CLI.

---

# Project status

`mvnex` is currently under active development.

The command surface and configuration formats may change before the first stable release.

---

# License

Copyright 2026 Sebastián Sánchez.

Licensed under the Apache License, Version 2.0. See [LICENSE](LICENSE) for details.
