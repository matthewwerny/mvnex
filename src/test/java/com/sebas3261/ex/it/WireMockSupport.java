package com.sebas3261.ex.it;

import static com.github.tomakehurst.wiremock.core.WireMockConfiguration.options;

import com.github.tomakehurst.wiremock.WireMockServer;
import java.io.File;
import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;

/**
 * Starts and stops a WireMock server for an integration test project from the
 * maven-invoker-plugin pre- and post-build hook scripts.
 *
 * <p>The hook scripts may run in different class loaders, so the running server
 * is kept as a JDK {@link Runnable} in the system properties rather than in a
 * static field. Stubs are loaded from {@code <basedir>/wiremock/mappings} and
 * {@code <basedir>/wiremock/__files}; the chosen port is written to
 * {@code <basedir>/wiremock.port}.
 */
public final class WireMockSupport {

    private static final String STOPPER_KEY_PREFIX = "ex.it.wiremock.stopper.";

    private WireMockSupport() {
    }

    public static int start(File basedir) throws IOException {
        WireMockServer server = new WireMockServer(options()
                .dynamicPort()
                .usingFilesUnderDirectory(new File(basedir, "wiremock").getAbsolutePath()));
        server.start();
        Runnable stopper = server::stop;
        System.getProperties().put(STOPPER_KEY_PREFIX + basedir.getAbsolutePath(), stopper);
        Files.writeString(new File(basedir, "wiremock.port").toPath(),
                Integer.toString(server.port()), StandardCharsets.US_ASCII);
        return server.port();
    }

    public static void stop(File basedir) {
        Object stopper = System.getProperties().remove(STOPPER_KEY_PREFIX + basedir.getAbsolutePath());
        if (stopper instanceof Runnable runnable) {
            runnable.run();
        }
    }
}
