import static com.sebas3261.ex.it.ItSupport.*
import static com.sebas3261.ex.it.WireMockSupport.*

def fq = { goal -> "com.sebas3261:ex-maven-plugin:${pluginVersion}:${goal}".toString() }

def dir = new File(basedir, 'work')
dir.mkdirs()
// Name, Group ID (Enter = default), Java version (number 3 = 17), Maven Wrapper (No)
mvn(dir, localRepositoryPath, ['-o', fq('init')], 'demo\n\n3\nNo\n', [:]).assertSuccess()
def pom = new File(dir, 'demo/pom.xml').getText('UTF-8')
assert pom.contains('<groupId>com.example</groupId>') && pom.contains('<maven.compiler.release>17</maven.compiler.release>')
assert !new File(dir, 'demo/mvnw').exists()

// End of input at the Group ID prompt cancels without creating anything.
def cancelled = new File(basedir, 'cancelled')
cancelled.mkdirs()
mvn(cancelled, localRepositoryPath, ['-o', fq('init')], 'demo\n', [:]).assertFailure().assertOutput('Operation cancelled.')
assert !new File(cancelled, 'demo').exists()
return true
