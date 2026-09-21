package com.sebas3261.ex.infrastructure.dependency;

import com.sebas3261.ex.application.errors.LookupNotPossibleException;
import java.util.List;

/**
 * Chooses the version of a versionless dependency deterministically, independent of which
 * search provider answered first (design D8):
 * <ol>
 *   <li>list all versions through the user's mirrors, in Maven version order;</li>
 *   <li>walk the non-pre-releases (or, if there are none, all versions) from the highest down and
 *       take the first one the public Central index confirms, which skips mirror-only builds;</li>
 *   <li>if confirmation fails or confirms nothing, take the highest of that pool;</li>
 *   <li>if the listing fails or is empty, keep the provider's provisional version.</li>
 * </ol>
 */
public final class CanonicalVersionSelector {

    private final RepositoryLookup repository;
    private final CentralIndex centralIndex;

    public CanonicalVersionSelector(RepositoryLookup repository, CentralIndex centralIndex) {
        this.repository = repository;
        this.centralIndex = centralIndex;
    }

    public String select(String groupId, String artifactId, String provisionalVersion) {
        List<String> versions;
        try {
            versions = VersionOrdering.ascending(repository.listVersions(groupId, artifactId));
        } catch (LookupNotPossibleException e) {
            throw e;
        } catch (RuntimeException e) {
            return provisionalVersion;
        }
        if (versions.isEmpty()) {
            return provisionalVersion;
        }

        List<String> stable = versions.stream().filter(version -> !PreRelease.isPreRelease(version)).toList();
        List<String> pool = stable.isEmpty() ? versions : stable;
        String highest = pool.get(pool.size() - 1);

        for (int i = pool.size() - 1; i >= 0; i--) {
            String version = pool.get(i);
            try {
                if (centralIndex.contains(groupId, artifactId, version)) {
                    return version;
                }
            } catch (LookupNotPossibleException e) {
                throw e;
            } catch (RuntimeException e) {
                return highest;
            }
        }
        return highest;
    }
}
