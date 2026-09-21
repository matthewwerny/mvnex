import static com.sebas3261.ex.it.ItSupport.*
import static com.sebas3261.ex.it.WireMockSupport.*

def fq = { goal -> "com.sebas3261:ex-maven-plugin:${pluginVersion}:${goal}".toString() }

// MAVEN_OPTS is word-split by the mvn script, so the home must not contain spaces.
def home = File.createTempDir()
def env = [MAVEN_OPTS: "-Duser.home=${home.absolutePath}".toString()]
mvn(basedir, localRepositoryPath, ['-o', fq('setup')], null, env)
        .assertSuccess().assertOutput('with plugin group com.sebas3261.')
def settings = new File(home, '.m2/settings.xml')
assert settings.getText('UTF-8').contains('<pluginGroup>com.sebas3261</pluginGroup>')
// The short prefix now resolves.
mvn(basedir, localRepositoryPath, ['-o', 'ex:help'], null, env).assertSuccess().assertOutput('This plugin has 5 goals')
return true
