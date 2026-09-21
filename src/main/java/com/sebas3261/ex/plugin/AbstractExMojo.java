package com.sebas3261.ex.plugin;

import com.sebas3261.ex.application.errors.DependencyResolutionException;
import com.sebas3261.ex.application.errors.OperationCancelledException;
import com.sebas3261.ex.application.ports.ReportSink;
import com.sebas3261.ex.plugin.support.LogReportSink;
import com.sebas3261.ex.plugin.support.ParameterGuard;
import java.io.UncheckedIOException;
import java.util.HashSet;
import java.util.Properties;
import java.util.Set;
import org.apache.maven.execution.MavenSession;
import org.apache.maven.plugin.AbstractMojo;
import org.apache.maven.plugin.MojoFailureException;
import org.apache.maven.plugin.descriptor.PluginDescriptor;
import org.apache.maven.plugins.annotations.Parameter;

/** Shared plumbing: the parameter guard, output, and mapping expected failures to build failures. */
abstract class AbstractExMojo extends AbstractMojo {

    /** The current Maven session. */
    @Parameter(defaultValue = "${session}", readonly = true, required = true)
    protected MavenSession session;

    /** This plugin's descriptor, for its version. */
    @Parameter(defaultValue = "${plugin}", readonly = true, required = true)
    protected PluginDescriptor plugin;

    /** Goal name as registered in the descriptor. */
    protected abstract String goal();

    /** The goal's work; expected failures are thrown as runtime exceptions carrying the user message. */
    protected abstract void run(ReportSink report);

    @Override
    public final void execute() throws MojoFailureException {
        ReportSink report = new LogReportSink(getLog());
        try {
            ParameterGuard.check(goal(), exPropertyKeys(), report);
            run(report);
        } catch (IllegalArgumentException | IllegalStateException | OperationCancelledException
                | DependencyResolutionException | UncheckedIOException e) {
            throw new MojoFailureException(e.getMessage(), e);
        }
    }

    protected String pluginVersion() {
        return plugin.getVersion();
    }

    /** A property from {@code -D}/{@code .mvn/maven.config} or the JVM, or null. */
    protected String property(String key) {
        String value = session.getUserProperties().getProperty(key);
        return value != null ? value : session.getSystemProperties().getProperty(key);
    }

    private Set<String> exPropertyKeys() {
        Set<String> keys = new HashSet<>();
        for (Properties properties : new Properties[] {session.getUserProperties(), session.getSystemProperties()}) {
            for (String key : properties.stringPropertyNames()) {
                if (key.startsWith("ex.")) {
                    keys.add(key);
                }
            }
        }
        return keys;
    }
}
