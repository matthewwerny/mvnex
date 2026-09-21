import static com.sebas3261.ex.it.ItSupport.*
import static com.sebas3261.ex.it.WireMockSupport.requests

try {
    // Solr says 1.18.38 (stale index) but the listing + deps.dev choose the current release.
    assertContains(new File(basedir, 'pom.xml'), '<version>1.18.48</version>')
    assertContains(new File(basedir, 'build.log'), 'org.projectlombok:lombok:1.18.48')
} finally {
    com.sebas3261.ex.it.WireMockSupport.stop(basedir)
}
return true
