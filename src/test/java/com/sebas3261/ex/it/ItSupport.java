package com.sebas3261.ex.it;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.Map;
import java.util.concurrent.TimeUnit;

/** Helpers for the maven-invoker-plugin hook scripts under {@code src/it}. */
public final class ItSupport {

    private ItSupport() {
    }

    /** A golden fixture from {@code src/test/resources/golden}. */
    public static byte[] golden(String path) throws IOException {
        try (InputStream in = ItSupport.class.getResourceAsStream("/golden/" + path)) {
            if (in == null) {
                throw new IOException("missing golden " + path);
            }
            return in.readAllBytes();
        }
    }

    public static String goldenText(String path) throws IOException {
        return new String(golden(path), StandardCharsets.UTF_8);
    }

    /** Asserts the file holds exactly the expected bytes. */
    public static void assertBytes(byte[] expected, File actual) throws IOException {
        byte[] bytes = Files.readAllBytes(actual.toPath());
        if (!Arrays.equals(expected, bytes)) {
            throw new AssertionError(actual + " differs from the expected bytes:\n--- expected\n"
                    + new String(expected, StandardCharsets.UTF_8) + "\n--- actual\n"
                    + new String(bytes, StandardCharsets.UTF_8));
        }
    }

    /** Asserts the file equals LF text whose line breaks are expanded to the platform separator. */
    public static void assertPlatformText(String lfText, File actual) throws IOException {
        assertBytes(lfText.replace("\n", System.lineSeparator()).getBytes(StandardCharsets.UTF_8), actual);
    }

    public static void assertContains(File file, String expected) throws IOException {
        String text = Files.readString(file.toPath(), StandardCharsets.UTF_8);
        if (!text.contains(expected)) {
            throw new AssertionError(file + " does not contain: " + expected);
        }
    }

    public static void assertNotContains(File file, String unexpected) throws IOException {
        String text = Files.readString(file.toPath(), StandardCharsets.UTF_8);
        if (text.contains(unexpected)) {
            throw new AssertionError(file + " unexpectedly contains: " + unexpected);
        }
    }

    /** Result of a nested Maven or process run. */
    public record Run(int exitCode, String output) {

        public Run assertSuccess() {
            if (exitCode != 0) {
                throw new AssertionError("expected success, exit " + exitCode + ":\n" + output);
            }
            return this;
        }

        public Run assertFailure() {
            if (exitCode == 0) {
                throw new AssertionError("expected failure:\n" + output);
            }
            return this;
        }

        public Run assertOutput(String expected) {
            if (!output.contains(expected)) {
                throw new AssertionError("output does not contain: " + expected + "\n" + output);
            }
            return this;
        }
    }

    /**
     * Runs the Maven that is running the invoker, in {@code dir}, against the IT local repository,
     * optionally feeding {@code stdin} (null for none) and extra environment variables.
     */
    public static Run mvn(File dir, File localRepository, List<String> args, String stdin, Map<String, String> env)
            throws IOException, InterruptedException {
        boolean windows = System.getProperty("os.name").toLowerCase().contains("win");
        File mavenHome = new File(System.getProperty("maven.home"));
        List<String> command = new ArrayList<>();
        command.add(new File(mavenHome, "bin/" + (windows ? "mvn.cmd" : "mvn")).getAbsolutePath());
        command.add("-ntp");
        command.add("-Dmaven.repo.local=" + localRepository.getAbsolutePath());
        command.addAll(args);
        return run(dir, command, stdin, env);
    }

    public static Run run(File dir, List<String> command, String stdin, Map<String, String> env)
            throws IOException, InterruptedException {
        ProcessBuilder builder = new ProcessBuilder(command).directory(dir).redirectErrorStream(true);
        builder.environment().putAll(env);
        Process process = builder.start();
        try (OutputStream in = process.getOutputStream()) {
            if (stdin != null) {
                in.write(stdin.getBytes(StandardCharsets.UTF_8));
            }
        }
        ByteArrayOutputStream out = new ByteArrayOutputStream();
        process.getInputStream().transferTo(out);
        if (!process.waitFor(10, TimeUnit.MINUTES)) {
            process.destroyForcibly();
            throw new AssertionError("timed out: " + command);
        }
        return new Run(process.exitValue(), out.toString(StandardCharsets.UTF_8));
    }

    /** The version of the Maven running the invoker (and therefore the invoked builds). */
    public static String mavenVersion() {
        File[] cores = new File(System.getProperty("maven.home"), "lib")
                .listFiles((dir, name) -> name.matches("maven-core-.+\\.jar"));
        if (cores == null || cores.length != 1) {
            throw new IllegalStateException("cannot determine the Maven version from maven.home");
        }
        String name = cores[0].getName();
        return name.substring("maven-core-".length(), name.length() - ".jar".length());
    }

    public static Path path(File file) {
        return file.toPath();
    }
}
