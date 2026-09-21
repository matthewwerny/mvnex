package com.sebas3261.ex.plugin;

import com.sebas3261.ex.application.add.AddUseCase;
import com.sebas3261.ex.application.errors.MultipleDependencyMatchesException;
import com.sebas3261.ex.application.ports.Interaction;
import com.sebas3261.ex.application.ports.MavenProjectValidator;
import com.sebas3261.ex.application.ports.ReportSink;
import com.sebas3261.ex.domain.dependency.DependencyRequest;
import com.sebas3261.ex.domain.dependency.ResolvedDependency;
import java.util.ArrayList;
import java.util.List;

/** The {@code ex:add} conversation: resolve, disambiguate, report. */
public final class AddFlow {

    static final String SEARCH_AGAIN = "Search again...";
    static final int VISIBLE_CANDIDATES = 3;

    private final Interaction interaction;
    private final ReportSink report;
    private final AddUseCase useCase;

    public AddFlow(Interaction interaction, ReportSink report, AddUseCase useCase) {
        this.interaction = interaction;
        this.report = report;
        this.useCase = useCase;
    }

    public void run(List<DependencyRequest> requests) {
        List<DependencyRequest> pending = new ArrayList<>(requests);
        AddUseCase.Result result;
        while (true) {
            try {
                result = useCase.execute(pending);
                break;
            } catch (MultipleDependencyMatchesException ambiguous) {
                int index = ambiguous.dependencyIndex();
                if (index < 0 || index >= pending.size()) {
                    throw ambiguous;
                }
                if (!interaction.isInteractive()) {
                    throw new IllegalStateException(batchAmbiguityMessage(ambiguous));
                }
                pending.set(index, choose(ambiguous, pending.get(index).scope()));
            }
        }
        report(result);
    }

    private DependencyRequest choose(MultipleDependencyMatchesException ambiguous, String scope) {
        List<ResolvedDependency> visible = visible(ambiguous);
        List<String> options = new ArrayList<>();
        for (ResolvedDependency candidate : visible) {
            options.add(candidate.coordinates());
        }
        options.add(SEARCH_AGAIN);

        String selected = interaction.select("Select dependency for " + ambiguous.query(), options, options.get(0));
        if (selected.equals(SEARCH_AGAIN)) {
            String term = interaction.text("Search dependency", ambiguous.query());
            return new DependencyRequest.SearchTerm(term.isEmpty() ? ambiguous.query() : term, scope);
        }
        ResolvedDependency chosen = visible.get(options.indexOf(selected));
        return new DependencyRequest.CoordinateWithVersion(chosen.groupId(), chosen.artifactId(), chosen.version(),
                scope);
    }

    static String batchAmbiguityMessage(MultipleDependencyMatchesException ambiguous) {
        StringBuilder message = new StringBuilder(ambiguous.getMessage());
        for (ResolvedDependency candidate : visible(ambiguous)) {
            message.append(System.lineSeparator()).append("  ").append(candidate.coordinates());
        }
        ResolvedDependency first = ambiguous.candidates().get(0);
        message.append(System.lineSeparator())
                .append("Specify an exact groupId:artifactId, for example -Dex.deps=")
                .append(first.groupId()).append(':').append(first.artifactId());
        return message.toString();
    }

    private static List<ResolvedDependency> visible(MultipleDependencyMatchesException ambiguous) {
        List<ResolvedDependency> candidates = ambiguous.candidates();
        return candidates.subList(0, Math.min(VISIBLE_CANDIDATES, candidates.size()));
    }

    private void report(AddUseCase.Result result) {
        if (!result.added().isEmpty()) {
            report.info("Dependencies added");
            for (ResolvedDependency dependency : result.added()) {
                report.info("  ✓ " + format(dependency));
            }
        }
        if (!result.skipped().isEmpty()) {
            report.info("Dependencies skipped");
            for (ResolvedDependency dependency : result.skipped()) {
                report.info("  - " + format(dependency) + " already exists or was duplicated in this command");
            }
        }
        if (result.added().isEmpty() && result.skipped().isEmpty()) {
            report.warn("No dependencies changed.");
        }
        result.validation().ifPresent(status -> {
            if (status == MavenProjectValidator.Status.PASSED) {
                report.info("Maven validate passed");
            } else {
                report.error("Maven validate failed. Check the project output with mvn validate.");
            }
        });
    }

    static String format(ResolvedDependency dependency) {
        return dependency.coordinates() + (dependency.scope().isEmpty() ? "" : " [" + dependency.scope() + "]");
    }
}
