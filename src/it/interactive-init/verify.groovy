import static com.sebas3261.ex.it.ItSupport.*
import static com.sebas3261.ex.it.WireMockSupport.*

def fq = { goal -> "com.sebas3261:ex-maven-plugin:${pluginVersion}:${goal}".toString() }
// CI machines turn interactive input off in several ways: Maven 3.9 switches to batch mode when CI=true,
// and a CI user settings file may set interactiveMode=false (actions/setup-java writes ~/.m2/settings.xml).
// Clear the variables and use settings that force interactive mode.
def interactive = [CI: null, MAVEN_ARGS: null]
def settings = new File(basedir, 'interactive-settings.xml')
settings.text = '<settings><interactiveMode>true</interactiveMode></settings>'

def dir = new File(basedir, 'work')
dir.mkdirs()
// Name, Group ID (Enter = default), Java version (number 3 = 17), Maven Wrapper (No)
mvn(dir, localRepositoryPath, ['-s', settings.absolutePath, fq('init')], 'demo\n\n3\nNo\n', interactive).assertSuccess()
def pom = new File(dir, 'demo/pom.xml').getText('UTF-8')
assert pom.contains('<groupId>com.example</groupId>') && pom.contains('<maven.compiler.release>17</maven.compiler.release>')
assert !new File(dir, 'demo/mvnw').exists()

// End of input at the Group ID prompt cancels without creating anything.
def cancelled = new File(basedir, 'cancelled')
cancelled.mkdirs()
mvn(cancelled, localRepositoryPath, ['-s', settings.absolutePath, fq('init')], 'demo\n', interactive).assertFailure().assertOutput('Operation cancelled.')
assert !new File(cancelled, 'demo').exists()
return true
