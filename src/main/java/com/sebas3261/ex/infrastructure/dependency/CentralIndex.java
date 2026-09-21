package com.sebas3261.ex.infrastructure.dependency;

/** The public Maven Central search index. */
public interface CentralIndex {

    /** Whether the index lists {@code groupId:artifactId:version}; throws if the index can't be queried. */
    boolean contains(String groupId, String artifactId, String version);
}
