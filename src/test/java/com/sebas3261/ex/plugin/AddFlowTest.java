package com.sebas3261.ex.plugin;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertThrows;
import static org.junit.jupiter.api.Assertions.assertTrue;

import com.sebas3261.ex.application.add.AddUseCase;
import com.sebas3261.ex.application.errors.MultipleDependencyMatchesException;
import com.sebas3261.ex.application.errors.OperationCancelledException;
import com.sebas3261.ex.application.ports.DependencyResolver;
import com.sebas3261.ex.application.ports.MavenProjectValidator;
import com.sebas3261.ex.application.ports.ProjectDependencyRepository;
import com.sebas3261.ex.domain.dependency.DependencyRequest;
import com.sebas3261.ex.domain.dependency.ResolvedDependency;
import java.util.ArrayList;
import java.util.List;
import java.util.Set;
import org.junit.jupiter.api.Test;

class AddFlowTest {

    private static final List<ResolvedDependency> LOMBOK_CANDIDATES = List.of(
            new ResolvedDependency("org.projectlombok", "lombok", "1.18.48"),
            new ResolvedDependency("io.github.valuya", "lombok", "1.18.46.4"),
            new ResolvedDependency("name.remal.gradle-plugins.lombok", "lombok", "3.2.0"),
            new ResolvedDependency("org.openrewrite.tools", "lombok", "1.0"));

    /** Ambiguous for "lombok"; everything else resolves to g:term:1.0. */
    private final List<String> resolverCalls = new ArrayList<>();
    private final DependencyResolver resolver = new DependencyResolver() {
        @Override
        public ResolvedDependency resolveBySearchTerm(String term, String version) {
            resolverCalls.add("term " + term + (version.isEmpty() ? "" : ":" + version));
            if (term.equals("lombok")) {
                throw new MultipleDependencyMatchesException(term, LOMBOK_CANDIDATES);
            }
            return new ResolvedDependency("org.example", term, version.isEmpty() ? "1.0" : version);
        }

        @Override
        public ResolvedDependency resolveByCoordinate(String groupId, String artifactId, String version) {
            resolverCalls.add("coordinate " + groupId + ":" + artifactId + ":" + version);
            return new ResolvedDependency(groupId, artifactId, version.isEmpty() ? "1.0" : version);
        }
    };

    private final List<List<ResolvedDependency>> writes = new ArrayList<>();
    private Set<String> existing = Set.of();
    private MavenProjectValidator.Status validation = MavenProjectValidator.Status.PASSED;

    private AddUseCase useCase() {
        ProjectDependencyRepository repository = new ProjectDependencyRepository() {
            @Override
            public Set<String> existingDependencyKeys() {
                return existing;
            }

            @Override
            public void addDependencies(List<ResolvedDependency> dependencies) {
                writes.add(dependencies);
            }
        };
        return new AddUseCase(resolver, repository, () -> validation);
    }

    private void run(ScriptedInteraction io, DependencyRequest... requests) {
        new AddFlow(io, io, useCase()).run(List.of(requests));
    }

    @Test
    void pickACandidate() {
        ScriptedInteraction io = new ScriptedInteraction(true, "org.projectlombok:lombok:1.18.48");
        run(io, new DependencyRequest.SearchTerm("lombok", ""));

        assertEquals(List.of("Select dependency for lombok [org.projectlombok:lombok:1.18.48, "
                + "io.github.valuya:lombok:1.18.46.4, name.remal.gradle-plugins.lombok:lombok:3.2.0, "
                + "Search again...] [org.projectlombok:lombok:1.18.48]"), io.prompts);
        assertEquals("org.projectlombok:lombok:1.18.48", writes.get(0).get(0).coordinates());
        assertTrue(resolverCalls.contains("coordinate org.projectlombok:lombok:1.18.48"));
    }

    @Test
    void chosenCandidateKeepsTheRequestedScope() {
        ScriptedInteraction io = new ScriptedInteraction(true, "");
        run(io, new DependencyRequest.SearchTerm("lombok", "provided"));

        assertEquals("provided", writes.get(0).get(0).scope());
    }

    @Test
    void searchAgainWithANewTerm() {
        ScriptedInteraction io = new ScriptedInteraction(true, "Search again...", "lombok-maven-plugin");
        run(io, new DependencyRequest.SearchTermWithVersion("lombok", "1.18.32", "test"));

        assertEquals("Search dependency [lombok]", io.prompts.get(1));
        ResolvedDependency added = writes.get(0).get(0);
        assertEquals("lombok-maven-plugin", added.artifactId());
        assertEquals("1.0", added.version(), "search again drops the explicit version");
        assertEquals("test", added.scope());
    }

    @Test
    void searchAgainWithEmptyAnswerRepeatsTheSearch() {
        ScriptedInteraction io = new ScriptedInteraction(true, "Search again...", "", "2");
        run(io, new DependencyRequest.SearchTerm("lombok", ""));

        assertEquals(3, io.prompts.size());
        assertTrue(io.prompts.get(2).startsWith("Select dependency for lombok"), "selector shown again");
        assertEquals("io.github.valuya", writes.get(0).get(0).groupId());
    }

    @Test
    void inputClosedAtTheSelectorCancels() {
        ScriptedInteraction io = new ScriptedInteraction(true);
        assertThrows(OperationCancelledException.class, () -> run(io, new DependencyRequest.SearchTerm("lombok", "")));
        assertTrue(writes.isEmpty());
    }

    @Test
    void batchAmbiguityFailsWithCandidatesAndHint() {
        IllegalStateException error = assertThrows(IllegalStateException.class,
                () -> run(ScriptedInteraction.batch(), new DependencyRequest.SearchTerm("lombok", "")));

        String nl = System.lineSeparator();
        assertEquals("Multiple dependency matches found: lombok" + nl
                + "  org.projectlombok:lombok:1.18.48" + nl
                + "  io.github.valuya:lombok:1.18.46.4" + nl
                + "  name.remal.gradle-plugins.lombok:lombok:3.2.0" + nl
                + "Specify an exact groupId:artifactId, for example -Dex.deps=org.projectlombok:lombok",
                error.getMessage());
        assertTrue(writes.isEmpty());
    }

    @Test
    void mixedResultOutput() {
        existing = Set.of("org.example:guava");
        ScriptedInteraction io = ScriptedInteraction.batch();
        run(io, new DependencyRequest.SearchTerm("guava", ""), new DependencyRequest.SearchTerm("junit-jupiter", "test"));

        assertEquals(List.of(
                "INFO Dependencies added",
                "INFO   ✓ org.example:junit-jupiter:1.0 [test]",
                "INFO Dependencies skipped",
                "INFO   - org.example:guava:1.0 already exists or was duplicated in this command",
                "INFO Maven validate passed"), io.output);
    }

    @Test
    void failedValidationIsLoggedAsErrorButDoesNotFail() {
        validation = MavenProjectValidator.Status.FAILED;
        ScriptedInteraction io = ScriptedInteraction.batch();
        run(io, new DependencyRequest.SearchTerm("guava", ""));

        assertTrue(io.output.contains("ERROR Maven validate failed. Check the project output with mvn validate."));
        assertEquals(1, writes.size());
    }

    @Test
    void nothingAddedMeansNoValidationOutput() {
        existing = Set.of("org.example:guava");
        ScriptedInteraction io = ScriptedInteraction.batch();
        run(io, new DependencyRequest.SearchTerm("guava", ""));

        assertTrue(io.output.stream().noneMatch(line -> line.contains("validate")));
    }
}
